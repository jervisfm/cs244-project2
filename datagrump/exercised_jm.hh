#ifndef EXERCISE_D_JM_CONTROLLER_HH
#define EXERCISE_D_JM_CONTROLLER_HH

#include <cmath>
#include <vector>
#include <map>

static const double HIGH_GAIN = 2.0 / log(2);

// Exercise D controller for Jervis' implementation.
// Tries to implement a BBR-like congestion controller which estimates
// RTT prop and Bottleneck Bandwidth to make sure window stays below Bandwidth Delay Product.
class ExDJMController : public Controller {
private:
    enum BBR_STATE {
     STARTUP,
     DRAIN,
     PROBE_BW,
     PROBE_RTT,
    };
    // State of bbr algorithm
    BBR_STATE bbr_state_;
    // Number of packets outstanding in the network.
    int inflight_packets_;
    // Value to control cwnd directly in startup mode.
    double cwnd_;
    // Factor to fudge BDP by.
    double cwnd_gain_;
    // Factor to fudge the delay by.
    double pacing_gain_;
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

  // Debugging test functions. Can be removed once we're confident in the implementation.
  void test_delivery_rates();

  inline bool mode_startup() {
    return bbr_state_ == BBR_STATE::STARTUP;
  }

  inline bool mode_probe_bw() {
    return bbr_state_ == BBR_STATE::PROBE_BW;
  }

  inline bool mode_probe_rtt() {
    return bbr_state_ == BBR_STATE::PROBE_RTT;
  }

  inline bool mode_drain() {
    return bbr_state_ == BBR_STATE::DRAIN;
  }

  inline void switch_to_mode_drain() {
    // Switch to drain mode. Keep the original cwnd gain inplace.
    bbr_state_ = BBR_STATE::DRAIN;
    pacing_gain_ = 1.0/HIGH_GAIN;
    cwnd_gain_ = HIGH_GAIN;
  }
  
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
