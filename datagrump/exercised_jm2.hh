#ifndef EXERCISE_D_JM2_CONTROLLER_HH
#define EXERCISE_D_JM2_CONTROLLER_HH

#include <cmath>
#include <vector>
#include <map>

// Default number of previous samples to consider for the sliding window.
static const int DEFAULT_WINDOW_NUM_SAMPLES = 10;

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
    // THe Min RTT when the previous ACK was received.
    double previous_min_rtt_;
    // The greatest sequence number successfully acked so far.
    uint64_t last_sequence_number_acked_;
    // Track whether on the previous ack, did we increase our cwnd ?
    bool did_increase_cwnd_;
    // Keeps track of all RTT samples we have seen so far.
    std::vector<double> rtt_samples_;
    // Keep track of how much in magnitude the min rtt samples are changing.
    std::vector<double> min_rtt_delta_samples_;
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
  double sliding_min_rtt(int num_samples=DEFAULT_WINDOW_NUM_SAMPLES);

  // Returns the maximum bandwidth over the past number of samples. Unit is bytes/msec
  double sliding_max_bandwidth(int num_samples=DEFAULT_WINDOW_NUM_SAMPLES);
  
  // Returns the very first RTT estimate which should give for an estimate on propagation delay.
  double rtt_min_initial_estimate();
  
  // Returns the maximum bandwidth over the past number of samples. Unit is kbytes/sec
  inline double sliding_max_bandwidth_kBs(int num_samples=DEFAULT_WINDOW_NUM_SAMPLES) {
    double bw_bytes_per_msec = sliding_max_bandwidth(num_samples);
    // Convert 1 bytes/msec -> 1 byte/sec
    double bw_bytes_per_sec = bw_bytes_per_msec / 0.001;

    // Convert 1 byte/sec -> 1kb/sec
    double bw_kilobytes_per_sec = bw_bytes_per_sec / 1024;

    return bw_kilobytes_per_sec;
  }

  // Returns the maximum bandwidth over the past number of samples. Unit is kbits/sec
  inline double sliding_max_bandwidth_kbs(int num_samples=DEFAULT_WINDOW_NUM_SAMPLES) {
    // Convert kBytes -> kiloBits
    return sliding_max_bandwidth_kBs(num_samples) * 8;
  }

  
  // Returns the current estimate for bandwidth delay product. This is a product
  // sliding_min_rtt() and sliding_max_bandwidth().
  double bandwidth_delay_product();

  // Computes the (estimated) inflight bandwidth delay product. Useful to know when
  // to swutch to PROBE_BW mode.
  double inflight_bdp() ;

  // Number of bytes sent in kilobytes.
  double num_bytes_sent_kb() {
    return num_bytes_sent_ / 1024.0;
  }

  // Number of bytes sent in megabytes.
  double num_bytes_sent_mb() {
    return num_bytes_sent_ / (1024 * 1024);
  }

  // BDP (nummber of bytes allowed to be outstanding on the network given current conditions
  // (RTT, BtlBw) in terms of outstanding packets.
  int bdp_packets();

  
  // BDP (nummber of bytes allowed to be outstanding on the network given current conditions
  // (RTT, BtlBw) in kilobytes.
  double bdp_kb() {
    return bandwidth_delay_product() / 1024.0;
  }

  // BDP (nummber of bytes allowed to be outstanding on the network given current conditions
  // (RTT, BtlBw) in megabytes.
  double bdp_mb() {
    return bandwidth_delay_product() / (1024 * 1024) ;
  }

  
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
