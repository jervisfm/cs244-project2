#ifndef EXERCISE_D_LH_CONTROLLER_HH
#define EXERCISE_D_LH_CONTROLLER_HH

// Exercise D controller for Luke's implementations/experiments.
class ExDLHController : public Controller {
private:
  unsigned int max_window_size_;
  unsigned int min_window_size_;
  double rtt_estimate_;
  double weight_;
  double ewma_rate_;
  unsigned int outstanding_packets_;

public:
  ExDLHController( const bool debug);

  unsigned int window_size() override;

  void datagram_was_sent( const uint64_t sequence_number,
                          const uint64_t send_timestamp,
                          bool on_timeout) override;

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
