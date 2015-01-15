void readAllBlocks (CommandsMessage &commandsMsg)
{
  uint16_t instructionBlocks1 = gpioExp1.readPort();
  uint16_t instructionBlocks2 = gpioExp2.readPort();
  uint16_t instructionBlocks3 = gpioExp3.readPort();
  uint16_t functionBlocks = gpioExp4.readPort();

  commandsMsg.mainInstructions[0] = ~instructionBlocks1 & PRIMO_BLOCK_INPUT_MASK;
  commandsMsg.mainInstructions[1] = ~(instructionBlocks1 >> 4) & PRIMO_BLOCK_INPUT_MASK;
  commandsMsg.mainInstructions[2] = ~(instructionBlocks1 >> 8) & PRIMO_BLOCK_INPUT_MASK;
  commandsMsg.mainInstructions[3] = ~(instructionBlocks1 >> 12) & PRIMO_BLOCK_INPUT_MASK;

  commandsMsg.mainInstructions[4] = swapBits(~instructionBlocks2);
  commandsMsg.mainInstructions[5] = swapBits(~instructionBlocks2 >> 4);
  commandsMsg.mainInstructions[6] = swapBits(~instructionBlocks2 >> 8);
  commandsMsg.mainInstructions[7] = rotateBits(~instructionBlocks2 >> 12);

  commandsMsg.mainInstructions[8] = ~instructionBlocks3 & PRIMO_BLOCK_INPUT_MASK;
  commandsMsg.mainInstructions[9] = ~(instructionBlocks3 >> 4) & PRIMO_BLOCK_INPUT_MASK;
  commandsMsg.mainInstructions[10] = ~(instructionBlocks3 >> 8) & PRIMO_BLOCK_INPUT_MASK;
  commandsMsg.mainInstructions[11] = ~(instructionBlocks3 >> 12) & PRIMO_BLOCK_INPUT_MASK;

  commandsMsg.functionInstructions[0] = ~functionBlocks & PRIMO_BLOCK_INPUT_MASK;
  commandsMsg.functionInstructions[1] = ~(functionBlocks >> 4) & PRIMO_BLOCK_INPUT_MASK;
  commandsMsg.functionInstructions[2] = ~(functionBlocks >> 8) & PRIMO_BLOCK_INPUT_MASK;
  commandsMsg.functionInstructions[3] = ~(functionBlocks >> 12) & PRIMO_BLOCK_INPUT_MASK;

  filterInvalidInstructions(commandsMsg);
}

////////////////////////////////////////////////////////////////////////////////

void filterInvalidInstructions (CommandsMessage &commandsMsg)
{
  int instrIdx = 0;
  
  while (instrIdx < PRIMO_MAX_MAIN_INSTRUCTIONS)
  {
    if (isKnownInstruction(commandsMsg.mainInstructions[instrIdx]))
      ++instrIdx;
    else
      break;
  }
  
  while (instrIdx < PRIMO_MAX_MAIN_INSTRUCTIONS)
    commandsMsg.mainInstructions[instrIdx++] = PRIMO_COMMAND_NONE;

  instrIdx = 0;
  
  while (instrIdx < PRIMO_MAX_FUNCTION_INSTRUCTIONS)
  {
    if (isKnownInstruction(commandsMsg.functionInstructions[instrIdx]))
      ++instrIdx;
    else
      break;
  }
  
  while (instrIdx < PRIMO_MAX_FUNCTION_INSTRUCTIONS)
    commandsMsg.functionInstructions[instrIdx++] = PRIMO_COMMAND_NONE;
}

////////////////////////////////////////////////////////////////////////////////

uint8_t swapBits (uint8_t value)
{
  // Swap bits 0 and 2 (bit 3 is preserved, all other bits are cleared)
  return ((value & 0x04) >> 2) | (value & 0x02) | ((value & 0x01) << 2);
}

////////////////////////////////////////////////////////////////////////////////

uint8_t rotateBits (uint8_t value)
{
  // Rotate the 3 LSBs (all other bits are cleared)
  return ((value & 0x06) >> 1) | ((value & 0x01) << 2);
}

////////////////////////////////////////////////////////////////////////////////

void writeLed (uint8_t ledNumber, uint8_t ledStatus)
{
  switch (ledNumber)
  {
    case 0 :
      gpioExp1.digitalWrite(3, ledStatus);
      break;

    case 1 :
      gpioExp1.digitalWrite(7, ledStatus);
      break;

    case 2 :
      gpioExp1.digitalWrite(11, ledStatus);
      break;

    case 3 :
      gpioExp1.digitalWrite(15, ledStatus);
      break;

    case 4 :
      gpioExp2.digitalWrite(3, ledStatus);
      break;

    case 5 :
      gpioExp2.digitalWrite(7, ledStatus);
      break;

    case 6 :
      gpioExp2.digitalWrite(11, ledStatus);
      break;

    case 7 :
      gpioExp2.digitalWrite(15, ledStatus);
      break;

    case 8 :
      gpioExp3.digitalWrite(3, ledStatus);
      break;

    case 9 :
      gpioExp3.digitalWrite(7, ledStatus);
      break;

    case 10 :
      gpioExp3.digitalWrite(11, ledStatus);
      break;

    case 11 :
      gpioExp3.digitalWrite(15, ledStatus);
      break;

    case 12 :
      gpioExp4.digitalWrite(3, ledStatus);
      break;

    case 13 :
      gpioExp4.digitalWrite(7, ledStatus);
      break;

    case 14 :
      gpioExp4.digitalWrite(11, ledStatus);
      break;

    case 15 :
      gpioExp4.digitalWrite(15, ledStatus);
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void switchAllLedsOn()
{
  for (int i = 0; i < 16;)
    writeLed(i++, PRIMO_LED_ON);
}

////////////////////////////////////////////////////////////////////////////////

void switchAllLedsOff()
{
  for (int i = 0; i < 16;)
    writeLed(i++, PRIMO_LED_OFF);
}

////////////////////////////////////////////////////////////////////////////////

void flashLed (uint8_t ledNumber, uint32_t flashingTime)
{
  uint32_t flashingStopMillis = millis() + flashingTime;
  uint32_t nextChangeMillis = 0UL;
  uint8_t ledStatus = PRIMO_LED_OFF;

  while(millis() < flashingStopMillis)
  {
    if (millis() > nextChangeMillis)
    {
      writeLed(ledNumber, ledStatus);

      ledStatus = (ledStatus == PRIMO_LED_ON ? PRIMO_LED_OFF : PRIMO_LED_ON);
    
      nextChangeMillis = millis() + PRIMO_LED_FLASHING_SEMIPERIOD;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void setAllLeds (CommandsMessage &commandsMsg)
{
  setMainLeds(commandsMsg);
  setFunctionLeds(commandsMsg);
}

////////////////////////////////////////////////////////////////////////////////

void setMainLeds(CommandsMessage &commandsMsg)
{
  for (int instrIdx = 0; instrIdx < PRIMO_MAX_MAIN_INSTRUCTIONS; ++instrIdx)
    writeLed(instrIdx, (isKnownInstruction(commandsMsg.mainInstructions[instrIdx]) ? PRIMO_LED_ON : PRIMO_LED_OFF));
}

////////////////////////////////////////////////////////////////////////////////

void setFunctionLeds(CommandsMessage &commandsMsg)
{
  for (int instrIdx = 0; instrIdx < PRIMO_MAX_FUNCTION_INSTRUCTIONS; ++instrIdx)
    writeLed(PRIMO_MAX_MAIN_INSTRUCTIONS + instrIdx, (isKnownInstruction(commandsMsg.functionInstructions[instrIdx]) ? PRIMO_LED_ON : PRIMO_LED_OFF));
}

////////////////////////////////////////////////////////////////////////////////

bool isKnownInstruction(uint8_t instrCode)
{
  switch (instrCode)
  {
    case PRIMO_COMMAND_RIGHT :
    case PRIMO_COMMAND_LEFT :
    case PRIMO_COMMAND_FORWARD :
    case PRIMO_COMMAND_FUNCTION :
      return true;

    default :
      return false;
  }
}

