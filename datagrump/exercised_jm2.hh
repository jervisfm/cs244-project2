#ifndef EXERCISE_D_JM2_CONTROLLER_HH
#define EXERCISE_D_JM2_CONTROLLER_HH

#include <cmath>
#include <vector>
#include <map>


// Exercise D controller for Jervis' implementation.
// A custom controller implemementation that's forked off an earlier attempt at BBR.
// Main idea is to try explore effect of just increasing Window whilst RRT is constant.
class ExDJM2Controller : public Controller {
private:
    // Number of packets outstanding in the network.
    int inflight_packets_;
    // Value to control cwnd directly in startup mode.
    double cwnd_;
    // Factor to fudge BDP by.
    double cwnd_gain_;
    // Total number data bytes sent.
    double num_bytes_sent_;
    // The greatest sequence number successfully acked so far.
    uint64_t last_sequence_number_acked_;
    // Keeps track of all RTT samples we have seen so far.
    std::vector<double> rtt_samples_;
    // Map of time -> how much data sent at that time.
    std::map<int, double> time_to_data_map_;

  // Returns the current average rtt
  double average_rtt();

  // Computes the delivery rate from the collected time_to_data_map_;
  // This is amount of data sent / time. The rates computed are in
  // time order.
  std::vector<double> delivery_rates();

  // Returns true if we saw substantial gains in delivery rate.
  bool delivery_rate_increased();
  
  // Returns the minimum rtt about the past number of samples. Unit is milliseconds.
  double sliding_min_rtt(int num_samples=10);

  // Returns the maximum bandwidth over the past number of samples. Unit is bytes/msec
  double sliding_max_bandwidth(int num_samples=10);
  
  // Returns the current estimate for bandwidth delay product. This is a product
  // sliding_min_rtt() and sliding_max_bandwidth().
  double bandwidth_delay_product();

  // Computes the (estimated) inflight bandwidth delay product. Useful to know when
  // to swutch to PROBE_BW mode.
  double inflight_bdp() ;
  
  // Debugging test functions. Can be removed once we're confident in the implementation.
  void test_delivery_rates();
  
public:
    ExDJM2Controller( const bool debug);

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
