#include <iostream>

#include "controller.hh"
#include "exercised.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
ExDController::ExDController( const bool debug )
  : Controller::Controller( debug ),
    cwnd_(10),
    alpha_(2.0),
    beta_(5.0),
    rtt_average_(50),
    rtt_allowance_(1.4),
    rtt_min_(80.0),
    ewma_weight_(0.01),
    timeout_(25),
    rtt_samples_() {
  cerr << "Exercise C" << endl;
}

double ExDController::sliding_min_rtt(int num_samples) {
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

/* Get current window size, in datagrams */
unsigned int ExDController::window_size( void ) {
  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
         << " window size is " << cwnd_ << endl;
  }
  // Simply return the cwnd_
  return (unsigned int) cwnd_;
}

/* A datagram was sent */
void ExDController::datagram_was_sent(  const uint64_t sequence_number, /* of the sent datagram */
                                        const uint64_t send_timestamp, /* in milliseconds */
                                        bool on_timeout ) {
  /* Default: take no action */
  if ( debug_ ) {
    cerr << "At time " << send_timestamp
         << " sent datagram " << sequence_number << "was timeout: " <<
    on_timeout << endl;
  }
}

/* An ack was received */
void ExDController::ack_received( const uint64_t sequence_number_acked,
                                  const uint64_t send_timestamp_acked,
                                  const uint64_t recv_timestamp_acked,
                                  const uint64_t timestamp_ack_received ) {
  // Calculate the RTT of the packet we just received
  double rtt_delay_ms = timestamp_ack_received - send_timestamp_acked;
  rtt_samples_.emplace_back(rtt_delay_ms);
  // Track the minimum RTT to use as a reference for RT_prop
  rtt_min_ = sliding_min_rtt(2000);

  // Keep an EWMA of the rtt seen so far.
  rtt_average_ = ewma_weight_ * rtt_delay_ms +
                 (1 - ewma_weight_) * rtt_average_;

  // If the current RTT is outside of the "stable" range of the average,
  // addtively decrease cwnd by beta, and reset the average to be RT_prop.
  if (rtt_delay_ms > (rtt_allowance_ *  rtt_average_)) {
    cwnd_ = max(cwnd_ - (beta_ / cwnd_), 1.0); // min cwnd = 1
    rtt_average_ = rtt_min_; // reset RTT
    if (debug_) {
      cerr << "      --- cwnd_:\t" << cwnd_
           << "\trtt_average_:\t" << rtt_average_
           << "\trtt:\t" << rtt_delay_ms << endl;
    }
  }
  // If the current RTT is within a stable range, continue to additively
  // increase the window size by alpha.
  else {
    cwnd_ += alpha_ / cwnd_;
    if (debug_) {
      cerr << "+++       cwnd_:\t" << cwnd_
           << "\trtt_average_:\t" << rtt_average_
           << "\trtt:\t" << rtt_delay_ms << endl;
    }
  }

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
         << " received ack for datagram " << sequence_number_acked
         << " (sent:" << send_timestamp_acked
         << ", received: " << recv_timestamp_acked << ")"
         << ", receive_ack: "  << timestamp_ack_received
         << ", rtt_delay_ms: " << rtt_delay_ms
         << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int ExDController::timeout_ms( void ) {
  return timeout_; /* timeout in milliseconds */
}
