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
  if (on_timeout) {
    // Multiplicative Decrease
    if ( debug_ ) {
      cerr << ">>> cwnd decrease from " << cwnd_
       << " to " << cwnd_ / beta_ << " using beta=" << beta_ << endl;
    }
    cwnd_ = cwnd_ / beta_;
  }
  if ( debug_ ) {
    cerr << "At time " << send_timestamp
     << " sent datagram " << sequence_number << "was timeout: " << on_timeout << endl;
  }
}

/* An ack was received */
void ExCController::ack_received( const uint64_t sequence_number_acked,
        /* what sequence number was acknowledged */
                                  const uint64_t send_timestamp_acked,
        /* when the acknowledged datagram was sent (sender's clock) */
                                  const uint64_t recv_timestamp_acked,
        /* when the acknowledged datagram was received (receiver's clock)*/
                                  const uint64_t timestamp_ack_received )
/* when the ack was received (by sender) */
{
  // Add alpha to cwnd on each ack
  cwnd_ += (double) alpha_ / cwnd_;
  if ( debug_ ) {
    cerr << ">>> cwnd increase from " << cwnd_ - ((double) alpha_ / cwnd_)
     << " to " << cwnd_  << " using alpha=" << alpha_ << endl;
  }
  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
         << " received ack for datagram " << sequence_number_acked
         << " (send @ time " << send_timestamp_acked
         << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
         << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int ExCController::timeout_ms( void )
{
  return 40; /* timeout in milliseconds */
}
