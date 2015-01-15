bool checkCommands (CommandsMessage &msgToCheck)
{
  for (int instrIdx = 0; instrIdx < PRIMO_MAX_MAIN_INSTRUCTIONS; ++instrIdx)
  {
    switch (msgToCheck.mainInstructions[instrIdx])
    {
      case PRIMO_COMMAND_NONE :
      case PRIMO_COMMAND_RIGHT :
      case PRIMO_COMMAND_LEFT :
      case PRIMO_COMMAND_FORWARD :
      case PRIMO_COMMAND_FUNCTION :
        break;

      default :
        return false;
    }
  }

  for (int instrIdx = 0; instrIdx < PRIMO_MAX_FUNCTION_INSTRUCTIONS; ++instrIdx)
  {
    switch (msgToCheck.functionInstructions[instrIdx])
    {
      case PRIMO_COMMAND_NONE :
      case PRIMO_COMMAND_RIGHT :
      case PRIMO_COMMAND_LEFT :
      case PRIMO_COMMAND_FORWARD :
        break;

      case PRIMO_COMMAND_FUNCTION :
      default :
        return false;
    }
  }
  
  return true;
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
//      case PRIMO_COMMAND_FORWARD :
//        leftStepper.move(-PRIMO_STEPPER_FORWARD_STEPS);
//        rightStepper.move(PRIMO_STEPPER_FORWARD_STEPS);
//        tone(PRIMO_BUZZER_PIN, 3000);
//        delay(50);
//        noTone(PRIMO_BUZZER_PIN);
//        oneOrMoreCommandsExecuted = true;
//        break;
//
//      case PRIMO_COMMAND_LEFT :
//        leftStepper.move(-PRIMO_STEPPER_TURN_STEPS);
//        rightStepper.move(-PRIMO_STEPPER_TURN_STEPS);
//        tone(PRIMO_BUZZER_PIN, 5000);
//        delay(50);
//        noTone(PRIMO_BUZZER_PIN);
//        oneOrMoreCommandsExecuted = true;
//        break;
//
//      case PRIMO_COMMAND_RIGHT :
//        leftStepper.move(PRIMO_STEPPER_TURN_STEPS);
//        rightStepper.move(PRIMO_STEPPER_TURN_STEPS);
//        tone(PRIMO_BUZZER_PIN, 4000);
//        delay(50);
//        noTone(PRIMO_BUZZER_PIN);
//        oneOrMoreCommandsExecuted = true;
//        break;

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
 
//    runBothSteppers();
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

