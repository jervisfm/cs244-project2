#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

// Configuration values
#define DEFAULT_TIME_WINDOW_MS 80.0
#define MAX_WINDOW_SIZE 50
#define MIN_WINDOW_SIZE 5
#define TRICKLE_WINDOW_SIZE 3
#define INIT_RATE 20
#define WEIGHT 0.25

// EWMA of the rate as seen by the receiver
double ewma_rate = INIT_RATE;

// The number of packets that have been sent, but not ACKed
uint64_t outstanding_packets = 0;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void ) {
  // Based on the current rate estimate, how many packets will leave the queue?
  // Here, I'm assuming the rate is 1 packet / ewma_rate, so multiplying by
  // a time window gives me an estimate of the number of packets leaving.
  uint32_t packets_leaving =
    (uint32_t) ((DEFAULT_TIME_WINDOW_MS / ewma_rate) + 0.5);
  uint32_t left_over_space = packets_leaving - outstanding_packets;
  if (outstanding_packets > packets_leaving) {
    left_over_space = TRICKLE_WINDOW_SIZE; // queue isn't drained fast enough!
  }
  else if (left_over_space < MIN_WINDOW_SIZE) {
    left_over_space = MIN_WINDOW_SIZE;
  }
  else if (left_over_space > MAX_WINDOW_SIZE) {
    left_over_space = MAX_WINDOW_SIZE;
  }

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
         << " window size is " << left_over_space << endl;
  }

  return left_over_space;
}

/**
 * A datagram was sent.
 * @param sequence_number of the sent datagram
 * @param send_timestamp  in milliseconds
 */
void Controller::datagram_was_sent( const uint64_t sequence_number,
                                    const uint64_t send_timestamp ) {
  if ( debug_ ) {
    cerr << "At time " << send_timestamp
         << " sent datagram " << sequence_number << endl;
  }

  outstanding_packets++; // track how many outstanding packets we have
}

/**
 * An ACK was received.
 * @param sequence_number_acked  what sequence number was acknowledged
 * @param send_timestamp_acked   when the acknowledged datagram was sent (sender's clock)
 * @param recv_timestamp_acked   when the acknowledged datagram was received (receiver's clock)
 * @param timestamp_ack_received when the ack was received (by sender)
 */
void Controller::ack_received( const uint64_t sequence_number_acked,
                               const uint64_t send_timestamp_acked,
                               const uint64_t recv_timestamp_acked,
                               const uint64_t timestamp_ack_received ) {
  static uint64_t last_recv_timestamp = recv_timestamp_acked;
  outstanding_packets--; // 1 packet has left the system

  // Calculate the EWMA of the rate seen at receiver.
  double current_rate = recv_timestamp_acked - last_recv_timestamp;
  ewma_rate = (WEIGHT * current_rate) + (1 - WEIGHT) * ewma_rate;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
         << " received ack for datagram " << sequence_number_acked
         << " (send @ time " << send_timestamp_acked
         << ", received @ time " << recv_timestamp_acked
         << " by receiver's clock)" << endl;
  }

  // Remember when the last ACK was
  last_recv_timestamp = recv_timestamp_acked;
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void ) {
  return 1000; /* timeout of one second */
}
