// Primo - Cubetto Playset
//    Bare-bones code for the Interface

// Sends a sequence of movement commands by radio, when the user-button is
// pressed.

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
#include <MCP23S17.h>
#include <limits.h>

#include "Primo.h"

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

// Flags to signal between interrupt handler and main event loop
volatile bool sendingMessage = false;
volatile bool ackReceived = false;

#define PRIMO_ACK_RX_TIMEOUT 2000UL

////////////////////////////////////////////////////////////////////////////////

// User buttons input pin (all buttons are connected in parallel)
#define PRIMO_USER_BUTTON_PIN 3

// Define SS pins for GPIO expanders
#define PRIMO_GPIOEXP1_SS_PIN 9
#define PRIMO_GPIOEXP2_SS_PIN 10
#define PRIMO_GPIOEXP3_SS_PIN 11
#define PRIMO_GPIOEXP4_SS_PIN 12

// Instantiate GPIO expanders objects
MCP23S17 gpioExp1(&SPI, PRIMO_GPIOEXP1_SS_PIN, 0);
MCP23S17 gpioExp2(&SPI, PRIMO_GPIOEXP2_SS_PIN, 0);
MCP23S17 gpioExp3(&SPI, PRIMO_GPIOEXP3_SS_PIN, 0);
MCP23S17 gpioExp4(&SPI, PRIMO_GPIOEXP4_SS_PIN, 0);

////////////////////////////////////////////////////////////////////////////////

#define PRIMO_BLOCK_INPUT_MASK 0x07

#define PRIMO_LED_ON  0
#define PRIMO_LED_OFF 1

#define PRIMO_LED_FLASHING_SEMIPERIOD 250UL

////////////////////////////////////////////////////////////////////////////////

void setup()
{
  // Set user buttons pin as an input
  pinMode(PRIMO_USER_BUTTON_PIN, INPUT);

  // Set up all the GPIO expanders
  gpioExp1.begin();
  gpioExp2.begin();
  gpioExp3.begin();
  gpioExp4.begin();
  
  // Set output direction for all the instruction blocks LED pins
  gpioExp1.pinMode(3, OUTPUT);
  gpioExp1.pinMode(7, OUTPUT);
  gpioExp1.pinMode(11, OUTPUT);
  gpioExp1.pinMode(15, OUTPUT);
  gpioExp2.pinMode(3, OUTPUT);
  gpioExp2.pinMode(7, OUTPUT);
  gpioExp2.pinMode(11, OUTPUT);
  gpioExp2.pinMode(15, OUTPUT);
  gpioExp3.pinMode(3, OUTPUT);
  gpioExp3.pinMode(7, OUTPUT);
  gpioExp3.pinMode(11, OUTPUT);
  gpioExp3.pinMode(15, OUTPUT);
  gpioExp4.pinMode(3, OUTPUT);
  gpioExp4.pinMode(7, OUTPUT);
  gpioExp4.pinMode(11, OUTPUT);
  gpioExp4.pinMode(15, OUTPUT);

  switchAllLedsOff(); 

  Serial.begin(115200);
  printf_begin();
  debugPrintf("Cubetto Playset - Interface\n\rVersion %s\n\r", PRIMO_CUBETTO_PLAYSET_VERSION);

  //
  // Session UID initialization
  //

  // Set the random number that will be used to uniquely identify THIS
  // communication session.  Note that random() actually returns a
  // pseudo-random sequence.  randomSeed() ensures that each device initialises
  // its session ID to a fairly random noise source
  randomSeed(analogRead(0) + analogRead(1) + analogRead(2) + analogRead(3) + analogRead(4) + analogRead(5));
  sessionId = random(1, LONG_MAX);

  //
  // Setup and configure RF radio module
  //
 
  radio.begin();
  //radio.setPALevel(RF24_PA_LOW);

  // Use the ACK payload feature (ACK payloads are dynamic payloads)
  radio.enableAckPayload();
  radio.enableDynamicPayloads();

  // Open pipes to other node for communication
  radio.openWritingPipe(PRIMO_INTERFACE2CUBETTO_PIPE_ADDRESS);
  radio.openReadingPipe(1, PRIMO_CUBETTO2INTERFACE_PIPE_ADDRESS); 

  // Dump the configuration of the RF module for debugging
  radio.printDetails();
  delay(50);

  // Attach interrupt handler to interrupt #1 (using pin 2 on Arduino Leonardo)
  attachInterrupt(1, checkRadio, LOW);

  //
  // Flash all LEDs three times
  //

  for (int ledLoop = 0; ledLoop < 3; ++ledLoop)
  {
    // Switch all LEDs on, delay for 100ms, then off
    if (ledLoop) delay(100);
    switchAllLedsOn(); 
    delay(100);
    switchAllLedsOff(); 
  }
}

////////////////////////////////////////////////////////////////////////////////

int buttonStatus,
    prevButtonStatus = HIGH;

////////////////////////////////////////////////////////////////////////////////

void loop()
{
  if (sendingMessage)
    return;

  CommandsMessage commandsMsg;

  readAllBlocks(commandsMsg);
  setAllLeds(commandsMsg);

  buttonStatus = digitalRead(PRIMO_USER_BUTTON_PIN);

  // Check if the user button has been pressed
  if ((prevButtonStatus == HIGH) && (buttonStatus == LOW))
  {
    // Debounce delay
    delay(100);
    
    // Check if the user button is still pressed
    if ((buttonStatus = digitalRead(PRIMO_USER_BUTTON_PIN)) == LOW)
    {
      sendCommandsToCubetto(commandsMsg);

      uint32_t timeoutMillis = millis() + PRIMO_ACK_RX_TIMEOUT;
      for(;;)
      {
        if (millis() > timeoutMillis)
          break;

        if (ackReceived)
        {
          simulateInstructions(commandsMsg);
          break;
        }
      }
    }
  }
      
  prevButtonStatus = buttonStatus;
}

