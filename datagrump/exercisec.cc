#include <iostream>

#include "controller.hh"
#include "timestamp.hh"
#include "exercisec.hh"


using namespace std;

/* Default constructor */
ExCController::ExCController( const bool debug )
  : Controller::Controller( debug ), cwnd_(10), alpha_(1), beta_(2), rtt_thresh_ms_(10)
{
    cerr << "Exercise C" << endl;
}

/* Get current window size, in datagrams */
unsigned int ExCController::window_size( void )
{
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
                                        bool on_timeout )
{
  /* Default: take no action */
  if ( debug_ ) {
    cerr << "At time " << send_timestamp
     << " sent datagram " << sequence_number << "was timeout: " << on_timeout << endl;
  }
}

/* An ack was received */
void ExCController::ack_received( const uint64_t sequence_number_acked,
                                  const uint64_t send_timestamp_acked,
                                  const uint64_t recv_timestamp_acked,
                                  const uint64_t timestamp_ack_received )

{
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

  if (rtt_delay_ms > rtt_thresh_ms_) {
    double diff_ms = rtt_delay_ms - rtt_thresh_ms_;
    if (debug_) {
      cerr << "<<< Exceeded RTT Threshold by " << diff_ms << "ms. Reducing Window size" << endl;
      cwnd_ -= 1;
      cwnd_ = std::max(cwnd_, 1);
    }
  } else {
    if (debug_) {
      cerr << "Still within RTT threshold limits, increasing cwnd" << endl;
      cwnd_ += 1;
      cwnd_ = std::max(cwnd_, 1);
    }
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int ExCController::timeout_ms( void )
{
  return 40; /* timeout in milliseconds */
}
