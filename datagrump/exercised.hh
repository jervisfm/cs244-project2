#ifndef EXERCISE_D_CONTROLLER_HH
#define EXERCISE_D_CONTROLLER_HH

class ExDController : public Controller {

private:
    double cwnd_;
    double alpha_;
    double beta_;
    double rtt_average_;
    // RTT Threshold to adjust cwnd.
    double rtt_allowance_;
    double rtt_min_;
    double ewma_weight_;
public:
    ExDController( const bool debug);

    unsigned int window_size() override;

    void datagram_was_sent( const uint64_t sequence_number,
                            const uint64_t send_timestamp,
                            bool on_timeout) override ;

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
