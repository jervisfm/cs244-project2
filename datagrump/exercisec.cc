#include <iostream>

#include "controller.hh"
#include "exercisec.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
ExCController::ExCController( const bool debug )
  : Controller::Controller( debug ), cwnd_(10), alpha_(2.0), beta_(5.0),
    rtt_thresh_ms_(50),
    rtt_min_(80.0),
    weight_(0.01) {
  cerr << "Exercise C" << endl;
}

/* Get current window size, in datagrams */
unsigned int ExCController::window_size( void ) {
  /* Default: fixed window size of 100 outstanding datagrams */
  // unsigned int the_window_size = 50;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
         << " window size is " << cwnd_ << endl;
  }

  return (unsigned int) cwnd_;
}

/* A datagram was sent */
void ExCController::datagram_was_sent(  const uint64_t sequence_number, /* of the sent datagram */
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
void ExCController::ack_received( const uint64_t sequence_number_acked,
                                  const uint64_t send_timestamp_acked,
                                  const uint64_t recv_timestamp_acked,
                                  const uint64_t timestamp_ack_received ) {
  // TODO: Increse window size +1) when RTT above rtt_thresh_ms_ and
  // reduce it by -1 when RTT below rtt_thresh_ms_
  double rtt_delay_ms = timestamp_ack_received - send_timestamp_acked;
  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
         << " received ack for datagram " << sequence_number_acked
         << " (sent:" << send_timestamp_acked
         << ", received: " << recv_timestamp_acked << ")"
         << ", receive_ack: "  << timestamp_ack_received
         << ", rtt_delay_ms: " << rtt_delay_ms
         << endl;
  }
  rtt_min_ = min(rtt_min_, rtt_delay_ms);
  rtt_thresh_ms_ = weight_ * rtt_delay_ms + (1 - weight_) * rtt_thresh_ms_;

  if (rtt_delay_ms > (1.4 *  rtt_thresh_ms_)) {
    double diff_ms = rtt_delay_ms - rtt_thresh_ms_;
    cwnd_ = max(cwnd_ - ((double) beta_ / cwnd_), 1.0); // min cwnd = 1
    // cwnd_ = max((cwnd_ - beta_), 1.0); // min cwnd = 1
    rtt_thresh_ms_ = rtt_min_; // reset RTT
    cerr << "      --- cwnd_:\t" << cwnd_
         << "\trtt_thresh_ms:\t" << rtt_thresh_ms_
         << "\trtt:\t" << rtt_delay_ms << endl;
    if (debug_) {
      cerr << "<<< Exceeded RTT Threshold by " << diff_ms <<
      "ms. Reducing Window size" << endl;
    }
  }
  else {
    if (debug_) {
      cerr << "Still within RTT threshold limits, increasing cwnd" << endl;
    }
    cwnd_ += (double) alpha_ / cwnd_;
    // cwnd_ += alpha_;
    cerr << "+++       cwnd_:\t" << cwnd_
         << "\trtt_thresh_ms:\t" << rtt_thresh_ms_
         << "\trtt:\t" << rtt_delay_ms << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int ExCController::timeout_ms( void ) {
  return 1000; /* timeout in milliseconds */
}
