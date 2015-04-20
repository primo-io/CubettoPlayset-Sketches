// Primo - Cubetto Playset
//    Bare-bones code for the Cubetto Robot

// Responds to a sequence of movement commands sent by radio.

// This code includes a mechanism for Interface/Cubetto Robot to pair.
// When a Cubetto Robot is powered-on, it is unpaired and will accept input from
// any Interface.  When an Interface is powered on, it generates a 32-bit random
// number to use as its session Unique ID (UID).
// Every radio message that an Interface sends includes this UID.
// A Cubetto Robot, on receiving its first radio message, records this UID,
// and subsequently ignores messages from any other UID.
// A devices's UID is forgotten when it is powered off.

// Normal usage of Interface/Cubetto Robot:
// 1. Power-on Cubetto Robot.
// 2. Send a message from the Interface.
// 3. These devices are now paired.
// (4). Repeat 1 & 2 for any other Interfaces & Cubetto Robots.
// (5). Power-off a Cubetto Robot to un-pair (the Interface is unaware of
//      pairings, so no need to power-off).

////////////////////////////////////////////////////////////////////////////////

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>
#include <AccelStepper.h>

#include "Primo.h"
#include "sound.h"


////////////////////////////////////////////////////////////////////////////////

#define PRIMO_DEBUG_MODE 

#ifdef PRIMO_DEBUG_MODE
#define debugPrintf printf
#define debugMessage dumpMessage
#else
#define debugPrintf(...) ((void) 0)
#define debugMessage(...) ((void) 0)
#endif

////////////////////////////////////////////////////////////////////////////////

// Set up nRF24L01 radio on SPI bus pins 7 (CE) and 8 (CSN)
RF24 radio(7, 8);

// Flag to signal between interrupt handler and main event loop
volatile bool noMessageReceived = true;

CommandsMessage commandsMsg;

////////////////////////////////////////////////////////////////////////////////

#define PRIMO_LEFT_STEPPER_PIN_1 12
#define PRIMO_LEFT_STEPPER_PIN_2 10
#define PRIMO_LEFT_STEPPER_PIN_3 11
#define PRIMO_LEFT_STEPPER_PIN_4 9

#define PRIMO_RIGHT_STEPPER_PIN_1 6
#define PRIMO_RIGHT_STEPPER_PIN_2 4
#define PRIMO_RIGHT_STEPPER_PIN_3 5
#define PRIMO_RIGHT_STEPPER_PIN_4 3

#define PRIMO_STEPPER_MAX_SPEED    1000
#define PRIMO_STEPPER_ACCELERATION 500

#define PRIMO_STEPPER_FORWARD_STEPS 2700
#define PRIMO_STEPPER_TURN_STEPS    1170

// Define a stepper and the pins it will use
AccelStepper leftStepper(AccelStepper::HALF4WIRE, PRIMO_LEFT_STEPPER_PIN_1, PRIMO_LEFT_STEPPER_PIN_2, PRIMO_LEFT_STEPPER_PIN_3, PRIMO_LEFT_STEPPER_PIN_4);
AccelStepper rightStepper(AccelStepper::HALF4WIRE, PRIMO_RIGHT_STEPPER_PIN_1, PRIMO_RIGHT_STEPPER_PIN_2, PRIMO_RIGHT_STEPPER_PIN_3, PRIMO_RIGHT_STEPPER_PIN_4);

//check inactivity and beep if inactive for too long
//time limit is 2 minutes (120 seconds)
long inactiveTimeLimit = 120000;
long timeStamp = 0;

////////////////////////////////////////////////////////////////////////////////

void setup()
{
  pinMode(PRIMO_BUZZER_PIN, OUTPUT);

  pinMode(PRIMO_LEFT_STEPPER_PIN_1, OUTPUT);
  pinMode(PRIMO_LEFT_STEPPER_PIN_2, OUTPUT);
  pinMode(PRIMO_LEFT_STEPPER_PIN_3, OUTPUT);
  pinMode(PRIMO_LEFT_STEPPER_PIN_4, OUTPUT);

  pinMode(PRIMO_RIGHT_STEPPER_PIN_1, OUTPUT);
  pinMode(PRIMO_RIGHT_STEPPER_PIN_2, OUTPUT);
  pinMode(PRIMO_RIGHT_STEPPER_PIN_3, OUTPUT);
  pinMode(PRIMO_RIGHT_STEPPER_PIN_4, OUTPUT);

  // IMPORTANT Stepper outputs are disabled here to minimise power usage whilst
  // stationary, as they are automatically enabled in the AccelStepper class
  // constructor
  leftStepper.disableOutputs();
  rightStepper.disableOutputs();

  leftStepper.setMaxSpeed(PRIMO_STEPPER_MAX_SPEED);
  rightStepper.setMaxSpeed(PRIMO_STEPPER_MAX_SPEED);
  leftStepper.setAcceleration(PRIMO_STEPPER_ACCELERATION);
  rightStepper.setAcceleration(PRIMO_STEPPER_ACCELERATION);

  Serial.begin(115200);
  printf_begin();
  debugPrintf("Cubetto Playset - Cubetto Robot\n\rVersion %s\n\r", PRIMO_CUBETTO_PLAYSET_VERSION);

  //
  // Setup and configure RF radio module
  //
 
  radio.begin();
  //radio.setPALevel(RF24_PA_LOW);

  // Use the ACK payload feature (ACK payloads are dynamic payloads)
  radio.enableAckPayload();
  radio.enableDynamicPayloads();

  // Open pipes to other node for communication
  radio.openWritingPipe(PRIMO_CUBETTO2INTERFACE_PIPE_ADDRESS);
  radio.openReadingPipe(1, PRIMO_INTERFACE2CUBETTO_PIPE_ADDRESS);

  radio.startListening();
 
  // Add an ACK packet for the next time around
  writeAckPayload();

  // Dump the configuration of the RF module for debugging
  radio.printDetails();
  delay(50);

  // Attach interrupt handler to interrupt #1 (using pin 2 on Arduino Leonardo)
  attachInterrupt(1, checkRadio, LOW);
  
  playPowerOnTune();
}

////////////////////////////////////////////////////////////////////////////////

void loop()
{
  if (noMessageReceived) {    
    checkInactivity();
    return;
  }
    
  timeStamp = millis();

  debugMessage(commandsMsg);

  executeInstructions(commandsMsg);

  noMessageReceived = true;
}

