void checkRadio(void)
{
  bool tx,fail,rx;

  radio.whatHappened(tx, fail, rx);
  
  // Have we successfully transmitted?
  if (tx)
  {
    debugPrintf("Message sent\n\r");
    sendingMessage = false;
  }

  // Have we failed to transmit?
  if (fail)
  {
    debugPrintf("Message transmission failed\n\r");
    sendingMessage = false;
  }
  
  // Did we receive a message?
  if (rx || radio.available())
  {
    // We've received an ACK payload
    AckMessage ackMsg;
    radio.read(&ackMsg, sizeof(ackMsg));
    
    debugPrintf("Got ACK\n\r  Sender ID: %lu\n\r  Session ID: %lu\n\r  Message counter: %lu\n\r", ackMsg.senderId, ackMsg.sessionId, ackMsg.messageCounter);

    if ((ackMsg.senderId == PRIMO_CUBETTO_ROBOT_ID) &&
        ((ackMsg.sessionId == sessionId) || ((ackMsg.sessionId == 0UL) && ((ackMsg.messageCounter == 0UL) || (ackMsg.messageCounter == 1UL)))))
      ackReceived = true;
  }
}

////////////////////////////////////////////////////////////////////////////////

void sendCommandsToCubetto(CommandsMessage &commandsMsg)
{
  commandsMsg.senderId = PRIMO_INTERFACE_ID;
  commandsMsg.sessionId = sessionId;

  uint8_t checksumAccumulator = 0x00;
  uint8_t *msgPtr = (uint8_t *) &commandsMsg;

  for (int idx = 0; idx < sizeof(commandsMsg) - sizeof(commandsMsg.checksum); ++idx)
    checksumAccumulator += *(msgPtr++);

  commandsMsg.checksum = checksumAccumulator;

  ackReceived = false;
  sendingMessage = true;
  radio.startWrite(&commandsMsg, sizeof(commandsMsg), 0);
}

