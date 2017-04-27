#include <iostream>
#include <sstream>
#include <cassert>
#include <limits>
#include <cmath>

#include "controller.hh"
#include "timestamp.hh"
#include "exercised_jm2.hh"


using namespace std;

// Size of the UDP packets we're sending.
static const int PACKET_SIZE_BYTES = 1472;



/* Default constructor */
ExDJM2Controller::ExDJM2Controller( const bool debug )
  : Controller::Controller( debug ),
    inflight_packets_(0),
    cwnd_(1),
    cwnd_gain_(0.8),
    num_bytes_sent_(0),
    previous_min_rtt_(0.0),
    last_sequence_number_acked_(0),
    did_increase_cwnd_(false),
    rtt_samples_(),
    min_rtt_delta_samples_(),
    time_to_data_map_()
          
{
  cerr << "Exercise D Jervis Custon (NonBBR) Controller" << endl;
}

int ExDJM2Controller::bdp_packets() {
  double bdp_outstanding_bytes = bandwidth_delay_product();
  double bdp_outstanding_packets = bdp_outstanding_bytes / PACKET_SIZE_BYTES;
  return bdp_outstanding_packets;
}

double ExDJM2Controller::rtt_min_initial_estimate() {
  if (rtt_samples_.empty()) {
    // Use a sensible default guess.
    return 40;
  } else {
    return rtt_samples_[0];
  }
}

/* Get current window size, in datagrams */
unsigned int ExDJM2Controller::window_size( void )
{
  int window_size = 0;

  // Try to Cap windows size to not grow beyond BDP.
  int bdp_outstanding_packets = bdp_packets();
  window_size = bdp_outstanding_packets;

  window_size = cwnd_;

  // Keep window size at least 1 to keep things moving.
  window_size = std::max(cwnd_, 1.0);
  
  //debug_printf(INFO, "At time %d, window size is %d", timestamp_ms(), window_size);
  debug_printf(VERBOSE, "At time %d, window size is %d", timestamp_ms(), window_size);


  
  return window_size;
  //return 100;
}

/* A datagram was sent */
void ExDJM2Controller::datagram_was_sent(  const uint64_t sequence_number, /* of the sent datagram */
                                        const uint64_t send_timestamp, /* in milliseconds */
                                        bool on_timeout )
{
  /* Default: take no action */
  debug_printf(VERBOSE, "At time %d sent datagram %d. Was timeout?%b", send_timestamp, sequence_number, on_timeout);
  ++inflight_packets_;
}

double ExDJM2Controller::average_rtt() {
  double total = 0;
  int count = 0;
  for (double value : rtt_samples_) {
    total += value;
    ++count;
  }
  return total/count;
}

double ExDJM2Controller::sliding_min_rtt(int num_samples) {

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

std::vector<double> ExDJM2Controller::delivery_rates() {
  std::vector<double> rates;
  // Loop over all the time -> num data sent samples in order and compute a vector
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

double ExDJM2Controller::bandwidth_delay_product() {
  return sliding_min_rtt() * sliding_max_bandwidth();
}

double ExDJM2Controller::sliding_max_bandwidth(int num_samples) {
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

void ExDJM2Controller::test_delivery_rates() { 
  time_to_data_map_ = {{1,10}, {2,20}, {3, 30}, {4,50}, {5,90}};
  stringstream output;
  for (const auto& rate : delivery_rates()) {
    output << rate << ", ";
  }
  debug_printf(INFO, "Rates: %s", output.str().c_str());
}

bool ExDJM2Controller::delivery_rate_increased() {
  std::vector<double> rates = delivery_rates();
  if (rates.size() <= 1) {
    // Be optimistic and assume rate is increasing if we don't have enough data.
    return true;
  }

  double current_value = rates[rates.size() - 1];
  double prior_value =  rates[rates.size() - 2];
  double change_magnitude = (current_value / prior_value);
  double change_threshold = 1.10;
  if (change_magnitude >= change_threshold) {
    return true;
  } else {
    return false;
  }  
}

double ExDJM2Controller::inflight_bdp() {
  double result = (PACKET_SIZE_BYTES / sliding_min_rtt()) * inflight_packets_;
  return result;
}

/* An ack was received */
void ExDJM2Controller::ack_received( const uint64_t sequence_number_acked,
                                  const uint64_t send_timestamp_acked,
                                  const uint64_t recv_timestamp_acked,
                                  const uint64_t timestamp_ack_received )
{
  --inflight_packets_;

  // Gather data for RTT estimates.
  double rtt_ms = timestamp_ack_received - send_timestamp_acked;
  rtt_samples_.emplace_back(rtt_ms);
  double average_rtt_ms = average_rtt();
  
  double rtt_change = sliding_min_rtt() - previous_min_rtt_;
  double rtt_change_percent = rtt_change / previous_min_rtt_ * 100.0;
  min_rtt_delta_samples_.emplace_back(rtt_change);
  previous_min_rtt_ = sliding_min_rtt();

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
  debug_printf(INFO, "Sliding Window Min RTT(ms): %.1f RTT delta (ms): %.1f (%.1f%%) Sliding Window Max Bandwidth (kbits/sec): %.1f, # Pkts inflight: %d # Kbytes sent: %.1f",
               sliding_min_rtt(),
               rtt_change,
               rtt_change_percent,
               sliding_max_bandwidth_kbs(),
               inflight_packets_,
               num_bytes_sent_kb());

  double original_cwnd_ = cwnd_;
  
  if ( sequence_number_acked == 0) {
    cwnd_ += 1;
  } else if (sequence_number_acked > 0)  {
    // Increase cwnd so long as RTT not spiking up and bandwidth increased.
    if (rtt_change_percent < 3) {
        debug_printf(INFO, "RTT still stable, growing window size.");
        //cwnd_ += 1/rtt_min_initial_estimate();
        cwnd_ += .23;
    } else {
      // RTT change increased abruptly, pull back cwnd
      if (did_increase_cwnd_) {
        debug_printf(WARN, "RTT jumped too much, pulling back cwnd.");
        cwnd_ *= 0.75;
      }
    }
      
  }

  
  // Track whether we increased cwnd in this ACK processing sessions.
  if (cwnd_ > original_cwnd_) {
    did_increase_cwnd_ = true;
  } else {
    did_increase_cwnd_ = false;
  }
  
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int ExDJM2Controller::timeout_ms( void )
{
  int timeout_ms = 2000; 
  return timeout_ms;
}
