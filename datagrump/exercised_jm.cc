#include <iostream>
#include <sstream>
#include <cassert>
#include <limits>
#include <cmath>

#include "controller.hh"
#include "timestamp.hh"
#include "exercised_jm.hh"


using namespace std;

// Size of the UDP packets we're sending.
static const int PACKET_SIZE_BYTES = 1472;



/* Default constructor */
ExDJMController::ExDJMController( const bool debug )
  : Controller::Controller( debug ),
    bbr_state_(BBR_STATE::STARTUP),
    inflight_packets_(0),
    cwnd_(1),
    cwnd_gain_(0.8),
    pacing_gain_(0.9),
    num_bytes_sent_(0),
    last_sequence_number_acked_(0),
    rtt_samples_(),
    time_to_data_map_()
          
{
  cerr << "Exercise D Jervis Controller" << endl;
}

/* Get current window size, in datagrams */
unsigned int ExDJMController::window_size( void )
{
  int window_size = 0;

    // BBR says that the window size should be some fraction of the BDP as modulated
    // by the cwnd gain.
    window_size = bandwidth_delay_product() * cwnd_gain_;

  
  // BBR recommends always have a min window size of 4 to keep things going smoothly.
  window_size = std::max(4, window_size);
  
  // window_size = 1;

  debug_printf(INFO, "At time %d, window size is %d", timestamp_ms(), window_size);

  return window_size;
}


/* A datagram was sent */
void ExDJMController::datagram_was_sent(  const uint64_t sequence_number, /* of the sent datagram */
                                        const uint64_t send_timestamp, /* in milliseconds */
                                        bool on_timeout )
{
  /* Default: take no action */
  debug_printf(VERBOSE, "At time %d sent datagram %d. Was timeout?%b", send_timestamp, sequence_number, on_timeout);
  ++inflight_packets_;
}

double ExDJMController::average_rtt() {
  double total = 0;
  int count = 0;
  for (double value : rtt_samples_) {
    total += value;
    ++count;
  }
  return total/count;
}

double ExDJMController::sliding_min_rtt(int num_samples) {

  double current_min_rtt = std::numeric_limits<double>::max();
  int count = 1;
  for(auto it = rtt_samples_.rbegin(); it != rtt_samples_.rend(); ++it) {
    if (count >= num_samples) {
      break;
    }
    double new_sample = *it;
    current_min_rtt = std::min(new_sample, current_min_rtt);
    ++count;
  }
  return current_min_rtt;
}

std::vector<double> ExDJMController::delivery_rates() {
  std::vector<double> rates;
  // Loop over all the time -> num data sent samples in order and compute a vectory
  // of delivery rates so far.
  int count = 0;
  std::pair<int, double> previous_entry;
  for (const auto& entry : time_to_data_map_) {
    ++count;
    if (count == 1) {
      previous_entry = entry;
      // We need two entries to compute a rate.
      continue;
    }

    // Pull out the needed data pieces from the two entries.
    int entry_time_sent = entry.first;
    double entry_data_sent = entry.second;

    int prev_entry_time_sent = previous_entry.first;
    double prev_entry_data_sent = previous_entry.second;

    // Compute intermediate deltas
    double delta_time = entry_time_sent - prev_entry_time_sent;
    double delta_data_sent = entry_data_sent - prev_entry_data_sent;

    // TODO: Remove debug assertion.
    assert(delta_time > 0);

    // Compute instantenous delivery rate from the deltas.
    double delivery_rate = delta_data_sent / delta_time;
    rates.emplace_back(delivery_rate);

    previous_entry = entry;
  }
  return rates;
}

double ExDJMController::bandwidth_delay_product() {
  return sliding_min_rtt() * sliding_max_bandwidth();
}

double ExDJMController::sliding_max_bandwidth(int num_samples) {
  std::vector<double> rates = delivery_rates();
  int count = 1;
  double current_max = 0;
  for (auto it = rates.rbegin(); it != rates.rend(); ++it) {
    if (count > num_samples) {
      break;
    }
    double new_sample = *it;
    current_max = std::max(new_sample, current_max);
    ++count;
  }
  return current_max;
}

void ExDJMController::test_delivery_rates() { 
  time_to_data_map_ = {{1,10}, {2,20}, {3, 30}, {4,50}, {5,90}};
  stringstream output;
  for (const auto& rate : delivery_rates()) {
    output << rate << ", ";
  }
  debug_printf(INFO, "Rates: %s", output.str().c_str());
}

bool ExDJMController::delivery_rate_increased() {
  std::vector<double> rates = delivery_rates();
  if (rates.size() <= 1) {
    // Be optimistic and assume rate is increasing if we don't have enough data.
    return true;
  }

  double current_value = rates[rates.size() - 1];
  double prior_value =  rates[rates.size() - 2];
  double change_magnitude = (current_value / prior_value);
  double change_threshold = 1.20;
  if (change_magnitude >= change_threshold) {
    return true;
  } else {
    return false;
  }  
}

/* An ack was received */
void ExDJMController::ack_received( const uint64_t sequence_number_acked,
                                  const uint64_t send_timestamp_acked,
                                  const uint64_t recv_timestamp_acked,
                                  const uint64_t timestamp_ack_received )
{
  --inflight_packets_;

  // Gather data for RTT estimates.
  double rtt_ms = timestamp_ack_received - send_timestamp_acked;
  rtt_samples_.emplace_back(rtt_ms);
  double average_rtt_ms = average_rtt();

  // Update stats for delivery rate.
  // TODO: Do we need to worry about out of order UDP packet delivery ? 
  if (sequence_number_acked > last_sequence_number_acked_) {
    // Got a new data point.
    last_sequence_number_acked_ = sequence_number_acked;
  }
  num_bytes_sent_ += PACKET_SIZE_BYTES;
  time_to_data_map_.emplace(timestamp_ack_received, num_bytes_sent_);
  
  debug_printf(VERBOSE, "At time=%d received ack for datagram=%d. Sent: %d. Receipt (recv's clock): %d  RTT(ms): %.1f Running Avg RTT(ms): %.1f",
               timestamp_ack_received, sequence_number_acked, send_timestamp_acked,
               recv_timestamp_acked, rtt_ms, average_rtt_ms);
  debug_printf(INFO, "Sliding Window Min RTT(ms): %.1f Sliding Window Max Bandwidth: %.1f", sliding_min_rtt(), sliding_max_bandwidth());


  // In start up mode, grow the cwnd exponetially.
  if (mode_startup()) {
    if (delivery_rate_increased()) {
      cwnd_gain_ = HIGH_GAIN;
      pacing_gain_ = HIGH_GAIN;
    } else {
      switch_to_mode_drain();
    }
    
  }

  
  
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int ExDJMController::timeout_ms( void )
{
  // This is equivalent to the timer callback delay in the BBR paper.
  // It's set to: PktSize / (pacing_gain * BtlBw)
  int timeout = PACKET_SIZE_BYTES / (pacing_gain_ * sliding_max_bandwidth());
  timeout = std::max(timeout, 1);
  debug_printf(VERBOSE, "Waiting for timeout (ms): %d", timeout);
  return timeout;
}
