void simulateInstructions(CommandsMessage &commandsMsg)
{
debugPrintf("simulateInstructions()");
  switchAllLedsOff();

  for (int instrIdx = 0; instrIdx < PRIMO_MAX_MAIN_INSTRUCTIONS; ++instrIdx)
  {
    uint32_t movementDelay = 0UL;

    switch (commandsMsg.mainInstructions[instrIdx])
    {
      case PRIMO_COMMAND_FUNCTION :
        writeLed(instrIdx, PRIMO_LED_ON);
        simulateFunction(commandsMsg);
        writeLed(instrIdx, PRIMO_LED_OFF);
        break;

      default :
        movementDelay = getMovementDelay(commandsMsg.mainInstructions[instrIdx]);
        break;
    }

    if (movementDelay)
    {
      writeLed(instrIdx, PRIMO_LED_ON);
      delay(movementDelay);
      writeLed(instrIdx, PRIMO_LED_OFF);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void simulateFunction(CommandsMessage &commandsMsg)
{
  for (int instrIdx = 0; instrIdx < PRIMO_MAX_FUNCTION_INSTRUCTIONS; ++instrIdx)
  {
    uint32_t movementDelay = getMovementDelay(commandsMsg.functionInstructions[instrIdx]);

    if (movementDelay)
    {
      writeLed(instrIdx + PRIMO_MAX_MAIN_INSTRUCTIONS, PRIMO_LED_ON);
      delay(movementDelay);
      writeLed(instrIdx + PRIMO_MAX_MAIN_INSTRUCTIONS, PRIMO_LED_OFF);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

uint32_t getMovementDelay(uint8_t command)
{
    switch (command)
    {
      case PRIMO_COMMAND_RIGHT :
        return 3000UL;

      case PRIMO_COMMAND_LEFT :
        return 3000UL;

      case PRIMO_COMMAND_FORWARD :
        return 4900UL;

      default :
        return 0UL;
    }
}

