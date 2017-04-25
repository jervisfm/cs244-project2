#ifndef DEFAULT_CONTROLLER_HH
#define DEFAULT_CONTROLLER_HH

#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

// Controller with default values as what was in original
// starter code.
class DefaultController : public Controller {
public:
    DefaultController( const bool debug);

    unsigned int window_size() override;

    void datagram_was_sent( const uint64_t sequence_number,
                           const uint64_t send_timestamp, bool on_timeout) override ;

    void ack_received( const uint64_t sequence_number_acked,
            /* what sequence number was acknowledged */
                              const uint64_t send_timestamp_acked,
            /* when the acknowledged datagram was sent (sender's clock) */
                              const uint64_t recv_timestamp_acked,
            /* when the acknowledged datagram was received (receiver's clock)*/
                              const uint64_t timestamp_ack_received ) override;

    unsigned int timeout_ms( void ) override;
};

#endif
