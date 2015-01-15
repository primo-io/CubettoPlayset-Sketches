void checkRadio()
{
  bool tx, fail, rx;

  radio.whatHappened(tx, fail, rx);

  // Have we successfully transmitted?
  if (tx)
  {
    debugPrintf("ACK payload sent\n\r");
  }

  // Have we failed to transmit?
  if (fail)
  {
    debugPrintf("ACK payload transmission failed\n\r");
  }

  // Did we receive a message?
  if (rx)
  {
    // Signal the main event loop that there is an Rx event to handle.
    rx_event_handler_pending = 1;

    // Get this payload and dump it
    radio.read(packet, PRIMO_NRF24L01_MAX_PACKET_SIZE);
    debugPrintf("Got payload\n\r");

    // Add an ack packet for the next time around.  This is a simple
    // packet counter
    radio.writeAckPayload(1, &ackMessageCount, sizeof(ackMessageCount));
    ++ackMessageCount;
  }
}

