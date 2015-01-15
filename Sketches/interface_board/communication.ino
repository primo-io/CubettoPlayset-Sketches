void checkRadio()
{
  bool tx, fail, rx;

  radio.whatHappened(tx, fail, rx);

  // Have we successfully transmitted?
  if (tx)
  {
    debugPrintf("Message sent\n\r");
  }

  // Have we failed to transmit?
  if (fail)
  {
    debugPrintf("Message transmission failed\n\r");
  }

  // Transmitter can power down for now, because
  // the transmission is done.
  if (tx || fail)
    radio.powerDown();

  // Did we receive a message?
  if (rx)
  {
    radio.read(&ackMessageCount, sizeof(ackMessageCount));
    debugPrintf("Got ACK: %lu\n\r", ackMessageCount);
  }
}

////////////////////////////////////////////////////////////////////////////////

void initialise_packet()
{
  long primo_id = PRIMO_ID;

  // Set the random number that will be used to uniquely identify THIS primo.
  // Note that random() actually returns a pseudo-random sequence.
  // randomSeed() ensures that each device initialises its sequence
  // to a fairly random noise source
  randomSeed(analogRead(0));
  sessionId = random();
  
  // Set the universal Primo ID, and the unique ID for this primo into the packet.
  // These values can be re-used in every packet sent.
  // (The UID could be used to set the packet address in the radio, but this would 
  // make it necessary to un-pair Primo/Cubetto at BOTH ends).
  memcpy(&packet[0], (const char*) &primo_id, 4);
  memcpy(&packet[4], (const char*) &sessionId, 4);

  // Test command to execute a sequence of 16 movement commands,
  // to produce a figure-of-8 pattern.
  packet[8] = PRIMO_COMMAND_FORWARD;
  packet[9] = PRIMO_COMMAND_LEFT;
  packet[10] = PRIMO_COMMAND_FORWARD;
  packet[11] = PRIMO_COMMAND_LEFT;
  packet[12] = PRIMO_COMMAND_FORWARD;
  packet[13] = PRIMO_COMMAND_LEFT;
  packet[14] = PRIMO_COMMAND_FORWARD;
  packet[15] = PRIMO_COMMAND_RIGHT;
  packet[16] = PRIMO_COMMAND_FORWARD;
  packet[17] = PRIMO_COMMAND_RIGHT;
  packet[18] = PRIMO_COMMAND_FORWARD;
  packet[19] = PRIMO_COMMAND_RIGHT;
  packet[20] = PRIMO_COMMAND_FORWARD;
  packet[21] = PRIMO_COMMAND_RIGHT;
  packet[22] = PRIMO_COMMAND_FORWARD;
  packet[23] = PRIMO_COMMAND_LEFT;

  //packet[8] = PRIMO_COMMAND_FORWARD;
  //packet[9] = PRIMO_COMMAND_FORWARD;
  //packet[10] = PRIMO_COMMAND_FORWARD;
  //packet[11] = PRIMO_COMMAND_FORWARD;
  //packet[12] = PRIMO_COMMAND_FORWARD;
  //packet[13] = PRIMO_COMMAND_FORWARD;
  //packet[14] = PRIMO_COMMAND_FORWARD;
  //packet[15] = PRIMO_COMMAND_FORWARD;
  //packet[16] = PRIMO_COMMAND_FORWARD;
  //packet[17] = PRIMO_COMMAND_FORWARD;
  //packet[18] = PRIMO_COMMAND_FORWARD;
  //packet[19] = PRIMO_COMMAND_FORWARD;
  //packet[20] = PRIMO_COMMAND_FORWARD;
  //packet[21] = PRIMO_COMMAND_FORWARD;
  //packet[22] = PRIMO_COMMAND_FORWARD;
  //packet[23] = PRIMO_COMMAND_FORWARD;

  // Ensure any unused positions are empty,
  // as Cubetto doesn't know if this is the end of the list
  // or just a gap before more movement instructions.
  packet[24] = PRIMO_COMMAND_STOP; 
  packet[25] = PRIMO_COMMAND_STOP;
  packet[26] = PRIMO_COMMAND_STOP;
  packet[27] = PRIMO_COMMAND_STOP;
  packet[28] = PRIMO_COMMAND_STOP;
  packet[29] = PRIMO_COMMAND_STOP;
  packet[30] = PRIMO_COMMAND_STOP;
  packet[31] = PRIMO_COMMAND_STOP;
}

