bool checkCommands (CommandsMessage &msgToCheck)
{
  for (int instrIdx = 0; instrIdx < PRIMO_MAX_MAIN_INSTRUCTIONS; ++instrIdx)
  {
    if (!checkValidCommand(msgToCheck.mainInstructions[instrIdx]))
      return false;
  }

  for (int instrIdx = 0; instrIdx < PRIMO_MAX_FUNCTION_INSTRUCTIONS; ++instrIdx)
  {
    if (!checkValidCommand(msgToCheck.functionInstructions[instrIdx]))
      return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool checkValidCommand(uint8_t cmdCode)
{
  switch (cmdCode)
  {
    case PRIMO_COMMAND_NONE :
    case PRIMO_COMMAND_RIGHT :
    case PRIMO_COMMAND_LEFT :
    case PRIMO_COMMAND_FORWARD :
    case PRIMO_COMMAND_FUNCTION :
      return true;

    default :
      return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool executeInstructions (CommandsMessage &commandsMsg)
{
  if (!checkCommands(commandsMsg))
  {
    playSadTune();

    return false;
  }

  bool oneOrMoreCommandsExecuted = false;

  for (int instrIdx = 0; instrIdx < PRIMO_MAX_MAIN_INSTRUCTIONS; ++instrIdx)
  {
    switch (commandsMsg.mainInstructions[instrIdx])
    {
      case PRIMO_COMMAND_FUNCTION :
        if (executeFunction(commandsMsg))
          oneOrMoreCommandsExecuted = true;
        break;

      default:
        if (setMovementAndBeep(commandsMsg.mainInstructions[instrIdx]))
        {
          runBothSteppers();
          oneOrMoreCommandsExecuted = true;
        }
        break;
    }
  }

  if (oneOrMoreCommandsExecuted)
    playHappyTune();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool executeFunction (CommandsMessage &commandsMsg)
{
  bool oneOrMoreCommandsExecuted = false;

  for (int instrIdx = 0; instrIdx < PRIMO_MAX_FUNCTION_INSTRUCTIONS; ++instrIdx)
  {
    if (commandsMsg.functionInstructions[instrIdx] == PRIMO_COMMAND_FUNCTION)
    {
      // Function recursive calls are allowed (infinite loop in our case)
      instrIdx = -1;
      continue;
    }

    if (setMovementAndBeep(commandsMsg.functionInstructions[instrIdx]))
    {
      runBothSteppers();
      oneOrMoreCommandsExecuted = true;
    }
  }

  return oneOrMoreCommandsExecuted;
}

////////////////////////////////////////////////////////////////////////////////

bool setMovementAndBeep(uint8_t command)
{
  bool moveCommandRecognized = false;

  switch (command)
  {
    case PRIMO_COMMAND_FORWARD :
      leftStepper.move(-PRIMO_STEPPER_FORWARD_STEPS);
      rightStepper.move(PRIMO_STEPPER_FORWARD_STEPS);
      tone(PRIMO_BUZZER_PIN, 3000);
      delay(50);
      noTone(PRIMO_BUZZER_PIN);
      moveCommandRecognized = true;
      break;

    case PRIMO_COMMAND_LEFT :
      leftStepper.move(-PRIMO_STEPPER_TURN_STEPS);
      rightStepper.move(-PRIMO_STEPPER_TURN_STEPS);
      tone(PRIMO_BUZZER_PIN, 5000);
      delay(50);
      noTone(PRIMO_BUZZER_PIN);
      moveCommandRecognized = true;
      break;

    case PRIMO_COMMAND_RIGHT :
      leftStepper.move(PRIMO_STEPPER_TURN_STEPS);
      rightStepper.move(PRIMO_STEPPER_TURN_STEPS);
      tone(PRIMO_BUZZER_PIN, 4000);
      delay(50);
      noTone(PRIMO_BUZZER_PIN);
      moveCommandRecognized = true;
      break;

    default:
      break;
  }

  return moveCommandRecognized;
}

////////////////////////////////////////////////////////////////////////////////

void runBothSteppers()
{
  leftStepper.enableOutputs();
  rightStepper.enableOutputs();

  bool leftStepperIsRunning, rightStepperIsRunning;

  do
  {
    // Run both steppers
    leftStepperIsRunning = leftStepper.run();
    rightStepperIsRunning = rightStepper.run();
  }
  while (leftStepper.run() || rightStepper.run());

  leftStepper.disableOutputs();
  rightStepper.disableOutputs();
}

/////////////////////////////////// INACTIVITY FUNCTIONS ////////////////////////////

void checkInactivity() {
  if (isInactive()) {
    inactiveSignal();
    timeStamp = millis();
  }
}

void inactiveSignal() {
  leftStepper.move(-PRIMO_STEPPER_TURN_STEPS / 2);
  rightStepper.move(-PRIMO_STEPPER_TURN_STEPS / 2);
  runBothSteppers();
  playSadTune();
  leftStepper.move(PRIMO_STEPPER_TURN_STEPS / 2);
  rightStepper.move(PRIMO_STEPPER_TURN_STEPS / 2);
  runBothSteppers();
}

boolean isInactive() {
  boolean b = false;

  long inactiveTime = (millis() - timeStamp);
  
  if (inactiveTime > inactiveTimeLimit) {
    b = true;
  } else {
    b = false;
  }
  return b;
}

