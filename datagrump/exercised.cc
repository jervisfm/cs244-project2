#include <iostream>

#include "controller.hh"
#include "timestamp.hh"
#include "exercised.hh"


using namespace std;

/* Default constructor */
ExDController::ExDController( const bool debug )
        : Controller::Controller( debug )
{}

/* Get current window size, in datagrams */
unsigned int ExDController::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = 50;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
         << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void ExDController::datagram_was_sent( const uint64_t sequence_number,
        /* of the sent datagram */
                                       const uint64_t send_timestamp )
/* in milliseconds */
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
         << " sent datagram " << sequence_number << endl;
  }
}

/* An ack was received */
void ExDController::ack_received( const uint64_t sequence_number_acked,
        /* what sequence number was acknowledged */
                                  const uint64_t send_timestamp_acked,
        /* when the acknowledged datagram was sent (sender's clock) */
                                  const uint64_t recv_timestamp_acked,
        /* when the acknowledged datagram was received (receiver's clock)*/
                                  const uint64_t timestamp_ack_received )
/* when the ack was received (by sender) */
{
  /* Default: take no action */

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
unsigned int ExDController::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
