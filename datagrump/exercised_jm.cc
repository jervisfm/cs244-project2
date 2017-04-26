#include <iostream>
#include <sstream>

#include "controller.hh"
#include "timestamp.hh"
#include "exercised_jm.hh"


using namespace std;


/* Default constructor */
ExDJMController::ExDJMController( const bool debug )
  : Controller::Controller( debug ), cwnd_gain_(0.8), rtt_samples_(), time_to_data_map_()
          
{
  cerr << "Exercise D Jervis Controller" << endl;
}

/* Get current window size, in datagrams */
unsigned int ExDJMController::window_size( void )
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
void ExDJMController::datagram_was_sent(  const uint64_t sequence_number, /* of the sent datagram */
                                        const uint64_t send_timestamp, /* in milliseconds */
                                        bool on_timeout )
{
  /* Default: take no action */
  debug_printf(VERBOSE, "At time %d sent datagram %d. Was timeout?%b", send_timestamp, sequence_number, on_timeout);

}

/* An ack was received */
void ExDJMController::ack_received( const uint64_t sequence_number_acked,
        /* what sequence number was acknowledged */
                                  const uint64_t send_timestamp_acked,
        /* when the acknowledged datagram was sent (sender's clock) */
                                  const uint64_t recv_timestamp_acked,
        /* when the acknowledged datagram was received (receiver's clock)*/
                                  const uint64_t timestamp_ack_received )
/* when the ack was received (by sender) */
{
  /* Default: take no action */

  double rtt_ms = timestamp_ack_received - send_timestamp_acked;
  debug_printf(INFO, "At time=%d received ack for datagram=%d. Sent: %d. Receipt (recv's clock): %d  RTT(ms): %.1f",
               timestamp_ack_received, sequence_number_acked, send_timestamp_acked,
               recv_timestamp_acked, rtt_ms);
  
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int ExDJMController::timeout_ms( void )
{
  return 1000; /* timeout of one second */
}
