#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>
// Include debug printf here so all controllers get debug functions.
#include "debug_printf.hh"

/* Congestion controller interface */

class Controller
{
protected:
  bool debug_; /* Enables debugging output */

  /* Add member variables here */

public:
  /* Public interface for the congestion controller */
  /* You can change these if you prefer, but will need to change
     the call site as well (in sender.cc) */

  /* Default constructor */
  Controller( const bool debug ): debug_(debug){}

  /* Default destructor */
  virtual ~Controller() = default;

  /* Get current window size, in datagrams */
  virtual unsigned int window_size( void ) = 0;

  /* A datagram was sent */
  virtual void datagram_was_sent( const uint64_t sequence_number,
			  const uint64_t send_timestamp, bool on_timeout ) = 0;

  /* Method called when An ack was received.
   * Arguments:
   * sequence_number_acked: what sequence number was acknowledged
   * send_timestamp_acked:  when the acknowledged datagram was sent (sender's clock)
   * recv_timestamp_acked:  when the acknowledged datagram was received (receiver's clock)
   * timestamp_ack_received:  when the ack was received (by sender).
   */
  virtual void ack_received( const uint64_t sequence_number_acked,
		     const uint64_t send_timestamp_acked,
		     const uint64_t recv_timestamp_acked,
		     const uint64_t timestamp_ack_received ) = 0;

  /* How long to wait (in milliseconds) if there are no acks
     before sending one more datagram */
  virtual unsigned int timeout_ms( void ) = 0;
};



#endif
