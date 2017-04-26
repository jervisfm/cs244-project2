#include <iostream>

#include "controller.hh"
#include "exercised_lh.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
ExDLHController::ExDLHController( const bool debug )
  : Controller::Controller( debug ),
    max_window_size_(80),
    min_window_size_(5),
    rtt_estimate_(80.0),
    weight_(0.3),
    ewma_rate_(2.4), // 1 packet per ewma_rate
    outstanding_packets_(0) {
  cerr << "Exercise D Luke Controller" << endl;
}

/* Get current window size, in datagrams */
unsigned int ExDLHController::window_size( void ) {
  // We use the estimated throughput seen at the receiver to estimate the
  // of packets that will leave the network in the next rtt_estimate_ ms.
  unsigned int packets_leaving =
    min((unsigned int)((rtt_estimate_ / ewma_rate_) + 0.5), max_window_size_);

  // To minimize the packets in the queue, we should send leftover_space
  // such that outstanding_packets_ + leftover_space = packets_leaving.
  unsigned int leftover_space = packets_leaving - outstanding_packets_;

  if (outstanding_packets_ > packets_leaving) {
    // If there are already too many packets, let them drain.
    return 0;
  }
  // else if (packets_leaving > max_window_size_) {
  //   // Put a maximum, just in case
  //   packets_leaving = max_window_size_;
  // }

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
         << " Estimated Packets Leaving: " << packets_leaving
         << " window size is " << leftover_space << endl;
  }

  cerr << "Outstanding Packets: " << outstanding_packets_
       << " Packets Leaving: " << packets_leaving
       << " Leftover: " << leftover_space
       << " rtt_estimate: " << rtt_estimate_
       << "\tewma: " << ewma_rate_ << endl;

  return max(packets_leaving, min_window_size_);
  // return leftover_space + outstanding_packets_;
}

/* A datagram was sent */
void ExDLHController::datagram_was_sent(  const uint64_t sequence_number, /* of the sent datagram */
                                          const uint64_t send_timestamp, /* in milliseconds */
                                          bool on_timeout ) {
  // Record that a datagram was sent
  outstanding_packets_++;

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
         << " sent datagram " << sequence_number << "was timeout: " <<
    on_timeout << endl;
  }
}

/* An ack was received */
void ExDLHController::ack_received( const uint64_t sequence_number_acked,
                                    /* what sequence number was acknowledged */
                                    const uint64_t send_timestamp_acked,
                                    /* when the acknowledged datagram was sent (sender's clock) */
                                    const uint64_t recv_timestamp_acked,
                                    /* when the acknowledged datagram was received (receiver's clock)*/
                                    const uint64_t timestamp_ack_received
                                    /* when the ack was received (by sender) */ )
{
  // Store the last time a packet was received by the receiver.
  static unsigned int last_recv_timestamp = recv_timestamp_acked;
  outstanding_packets_--; // record that a packet has left the system

  rtt_estimate_ =  min((double)(timestamp_ack_received - send_timestamp_acked),
                       rtt_estimate_);
  // On receipt of an ACK, estimate the EWMA of the rate seen at the receiver
  // as 1 packet / current_rate (ms)
  double current_rate = max((double)recv_timestamp_acked - last_recv_timestamp,
                            0.3);
  ewma_rate_ = (weight_ * current_rate) + (1.0 - weight_) * ewma_rate_;

  // Update the last received timestamp
  last_recv_timestamp = recv_timestamp_acked;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
         << " received ack for datagram " << sequence_number_acked
         << " (send @ time " << send_timestamp_acked
         << ", received @ time " << recv_timestamp_acked
         << " by receiver's clock)"
         << " ewma: " << ewma_rate_
         << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int ExDLHController::timeout_ms( void ) {
  return 1000; /* timeout of one second */
}
