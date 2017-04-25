#include <iostream>

#include "controller.hh"
#include "timestamp.hh"
#include "exerciseb.hh"


using namespace std;

/* Default constructor */
ExBController::ExBController( const bool debug )
        : Controller::Controller( debug ), cwnd_(10), alpha_(0.1), beta_(1.5)
{
    cerr << "Exercise B" << endl;
}

/* Get current window size, in datagrams */
unsigned int ExBController::window_size( void )
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
void ExBController::datagram_was_sent(  const uint64_t sequence_number, /* of the sent datagram */
                                        const uint64_t send_timestamp, /* in milliseconds */
                                        bool on_timeout )
{
  // We used a fixed timeout as the signal for multiplicative decrease.
  // Note that in our mahimahi environment, there is no loss, and there are
  // unbounded queues on the bottleneck path.
  if (on_timeout) {
    // Multiplicative Decrease: Divide cwnd by beta on timeout
    if ( debug_ ) {
      cerr << ">>> cwnd decrease from " << cwnd_
       << " to " << max(cwnd_ / beta_, 1.0) << " using beta=" << beta_ << endl;
    }
    cwnd_ = max(cwnd_ / beta_, 1.0); // min cwnd = 1
  }
  if ( debug_ ) {
    cerr << "At time " << send_timestamp
     << " sent datagram " << sequence_number << "was timeout: " << on_timeout << endl;
  }
}

/* An ack was received */
void ExBController::ack_received( const uint64_t sequence_number_acked,
        /* what sequence number was acknowledged */
                                  const uint64_t send_timestamp_acked,
        /* when the acknowledged datagram was sent (sender's clock) */
                                  const uint64_t recv_timestamp_acked,
        /* when the acknowledged datagram was received (receiver's clock)*/
                                  const uint64_t timestamp_ack_received )
/* when the ack was received (by sender) */
{
  // Additive Increase: Add alpha/cwnd to cwnd on each ack
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
unsigned int ExBController::timeout_ms( void )
{
  return 80; /* timeout in milliseconds */
}
