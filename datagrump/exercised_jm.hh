#ifndef EXERCISE_D_JM_CONTROLLER_HH
#define EXERCISE_D_JM_CONTROLLER_HH

#include <vector>
#include <map>

// Exercise D controller for Jervis' implementation.
class ExDJMController : public Controller {
private:
    // Factor to fudge BDP by.
    double cwnd_gain_;
    // Total number data bytes sent.
    double num_bytes_sent_;
    // Keeps track of all RTT samples we have seen so far.
    std::vector<double> rtt_samples_;
    // Map of time -> how much data sent at that time.
    std::map<int, double> time_to_data_map_;

  // Returns the current average rtt
  double average_rtt();

  // Computes the delivery rate from the collected time_to_data_map_;
  // This is amount of data sent / time.
  std::vector<double> delivery_rate();
  
  // Returns the minimum rtt about the past number of samples.
  double sliding_min_rtt(int num_samples=10);

  // Returns the maximum bandwidth over the past number of samples
  double sliding_max_bandwidth(int num_samples=10);
  
  // Returns the current estimate for bandwidth delay product. This is a product
  // sliding_min_rtt() and sliding_max_bandwidth().
  double bandwidth_delay_product();
  
public:
    ExDJMController( const bool debug);

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
