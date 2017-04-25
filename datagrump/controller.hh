#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>

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

  /* An ack was received */
  virtual void ack_received( const uint64_t sequence_number_acked,
		     const uint64_t send_timestamp_acked,
		     const uint64_t recv_timestamp_acked,
		     const uint64_t timestamp_ack_received ) = 0;

  /* How long to wait (in milliseconds) if there are no acks
     before sending one more datagram */
  virtual unsigned int timeout_ms( void ) = 0;
};



#endif
