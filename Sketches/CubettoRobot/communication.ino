void checkRadio()
{
  bool tx, fail, rx;

  radio.whatHappened(tx, fail, rx);

  // Have we successfully transmitted?
  if (tx)
    debugPrintf("ACK payload sent\n\r");

  // Have we failed to transmit?
  if (fail)
    debugPrintf("ACK payload transmission failed\n\r");

  // Did we receive a message?
  if (rx || radio.available())
  {
    if (noMessageReceived)
    {
      // We've received a message: get the payload
      radio.read(&commandsMsg, sizeof(commandsMsg));
     
      // Add an ACK packet for the next time around
      writeAckPayload();
  
      if (checkMessage(commandsMsg))
        noMessageReceived = false;
    }
    else
    {
      // A previous message execution is running: discard this message
      CommandsMessage dummyMsg;
      radio.read(&dummyMsg, sizeof(dummyMsg));

      // Add an ACK packet for the next time around
      writeAckPayload();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void writeAckPayload()
{
  AckMessage ackMsg;
  ackMsg.senderId = PRIMO_CUBETTO_ROBOT_ID;
  ackMsg.sessionId = sessionId;
  ackMsg.messageCounter = ackMessageCount++;

  radio.writeAckPayload(1, &ackMsg, sizeof(ackMsg));
}

////////////////////////////////////////////////////////////////////////////////

bool checkMessage (CommandsMessage &msgToCheck)
{
  if (msgToCheck.senderId != PRIMO_INTERFACE_ID)
    return false;

  if (sessionId != 0UL)
  {
    if (sessionId != msgToCheck.sessionId)
      return false;
  }
  else
  {
    sessionId = msgToCheck.sessionId;
    debugPrintf("Got new session ID: %lu\n\r", sessionId);
  }

  uint8_t checksumAccumulator = 0x00;
  uint8_t *msgPtr = (uint8_t *) &msgToCheck;

  for (int idx = 0; idx < sizeof(msgToCheck) - sizeof(msgToCheck.checksum); ++idx)
    checksumAccumulator += *(msgPtr++);

  return (msgToCheck.checksum == checksumAccumulator);
}

////////////////////////////////////////////////////////////////////////////////

void dumpMessage (CommandsMessage &msgToDump)
{
  debugPrintf("Sender ID: %lu\n\r", msgToDump.senderId);
  debugPrintf("Session ID: %lu\n\r", msgToDump.sessionId);

  debugPrintf("Main instructions:");
  for (int instrIdx = 0; instrIdx < PRIMO_MAX_MAIN_INSTRUCTIONS; ++instrIdx)
    debugPrintf(" %02X", msgToDump.mainInstructions[instrIdx]);
  debugPrintf("\n\r");

  debugPrintf("Function instructions:");
  for (int instrIdx = 0; instrIdx < PRIMO_MAX_FUNCTION_INSTRUCTIONS; ++instrIdx)
    debugPrintf(" %02X", msgToDump.functionInstructions[instrIdx]);
  debugPrintf("\n\r");

  debugPrintf("Checksum: %02X\n\r", msgToDump.checksum);

  debugPrintf("Message check: %s\n\r", (checkMessage(msgToDump) ? "OK" : "BAD!!!"));
  debugPrintf("Commands check: %s\n\r", (checkCommands(msgToDump) ? "OK" : "BAD!!!"));
}

