void move (void)
{
  int i;

  for (i = 8; i < 32; i++)
  {
    switch (packet[i])
    {
      case PRIMO_COMMAND_STOP:
        playHappyTune();
        break;

      case PRIMO_COMMAND_FORWARD:
        leftStepper.move(-PRIMO_STEPPER_FORWARD_STEPS);
        rightStepper.move(PRIMO_STEPPER_FORWARD_STEPS);
        tone(PRIMO_BUZZER_PIN, 3000);
        delay(50);
        noTone(PRIMO_BUZZER_PIN);
        break;

      case PRIMO_COMMAND_LEFT:
        leftStepper.move(-PRIMO_STEPPER_TURN_STEPS);
        rightStepper.move(-PRIMO_STEPPER_TURN_STEPS);
        tone(PRIMO_BUZZER_PIN, 5000);
        delay(50);
        noTone(PRIMO_BUZZER_PIN);
        break;

      case PRIMO_COMMAND_RIGHT:
        leftStepper.move(PRIMO_STEPPER_TURN_STEPS);
        rightStepper.move(PRIMO_STEPPER_TURN_STEPS);
        tone(PRIMO_BUZZER_PIN, 4000);
        delay(50);
        noTone(PRIMO_BUZZER_PIN);
        break;

      default:
        break;
    }

    leftStepperIsMoving = 1;
    rightStepperIsMoving = 1;

    while (leftStepperIsMoving || rightStepperIsMoving)
    {
      leftStepperIsMoving = leftStepper.run();
      rightStepperIsMoving = rightStepper.run();
    }
  }
}

