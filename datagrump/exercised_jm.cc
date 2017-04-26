#include <iostream>
#include <sstream>

#include "controller.hh"
#include "timestamp.hh"
#include "exercised_jm.hh"


using namespace std;

// Size of the UDP packets we're sending.
static const int PACKET_SIZE_BYTES = 1472;

/* Default constructor */
ExDJMController::ExDJMController( const bool debug )
  : Controller::Controller( debug ),
    cwnd_gain_(0.8),
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
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = 1;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
         << " window size is " << the_window_size << endl;
  }
  return the_window_size;
}


/* A datagram was sent */
void ExDJMController::datagram_was_sent(  const uint64_t sequence_number, /* of the sent datagram */
                                        const uint64_t send_timestamp, /* in milliseconds */
                                        bool on_timeout )
{
  /* Default: take no action */
  debug_printf(VERBOSE, "At time %d sent datagram %d. Was timeout?%b", send_timestamp, sequence_number, on_timeout);

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
  int count = 0;
  double total = 0;
  for(auto it = rtt_samples_.rbegin(); it != rtt_samples_.rend(); ++it) {
    if (count > num_samples) {
      break;
    }
    double value = *it;
    total += value;
    ++count;
  }
  return total/count;
}

std::vector<double> ExDJMController::delivery_rates() {
  std::vector<double> rates;
  // TODO: implement.
  return rates;
}

double ExDJMController::bandwidth_delay_product() {
  // TODO: implement.
  return 1;
}

/* An ack was received */
void ExDJMController::ack_received( const uint64_t sequence_number_acked,
                                  const uint64_t send_timestamp_acked,
                                  const uint64_t recv_timestamp_acked,
                                  const uint64_t timestamp_ack_received )
{
  /* Default: take no action */

  // Gather data for RTT estimates.
  double rtt_ms = timestamp_ack_received - send_timestamp_acked;
  rtt_samples_.emplace_back(rtt_ms);
  double average_rtt_ms = average_rtt();
  
  debug_printf(VERBOSE, "At time=%d received ack for datagram=%d. Sent: %d. Receipt (recv's clock): %d  RTT(ms): %.1f Running Avg RTT(ms): %.1f",
               timestamp_ack_received, sequence_number_acked, send_timestamp_acked,
               recv_timestamp_acked, rtt_ms, average_rtt_ms);
  debug_printf(INFO, "Sliding Window Min RTT(ms): %.1f", sliding_min_rtt());
  
  
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int ExDJMController::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
