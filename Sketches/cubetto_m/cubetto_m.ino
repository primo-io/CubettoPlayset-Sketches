// Bare-bones code for Cubetto.
// Responds to a sequence of movement commands sent by radio.
// (See also primo_dom. Bare-bones code to send movement commands).

// Uses Olimex 32u4 Leonardo boards, with nRf24l01 radio.

// Note that relaible radio performance requires that all connections to the radio use short wires
// and soldered joints.

// Pin-name Colour   Leonardo    nRF24L01

// MOSI     Blue     4(ICSP)     6
// MISO     Red      1(ICSP)     7
// SCK      Green    3(ICSP)     5
// CE       Orange   7(DIGITAL)  3
// CSN      Yellow   8(DIGITAL)  4
// IRQ      White    2(DIGITAL)  8
// GND      Black    GND(POWER)  1
// 3v3      Red      3v3(POWER)  2

// Note that these connections differ from the original RF24 library examples.
// (RF24 library must be installed).

// This code includes a mechanism for Primo/Cubetto to pair
// When a Cubetto is powered-on, it is unpaired and will accept input from any Primo.
// When a Primo is powered on, it generates a 32-bit random number to use as its Unique ID (UID).
// Every radio message that a Primo sends includes this UID.
// A Cubetto, on receiving its first radio message, records this UID,
// and subsequently ignores messages from any other UID.
// A devices's UID is forgotten when it is powered off.

// Normal usage of Primo/Cubetto:
// 1. Power-on Cubetto.
// 2. Send a message from Primo.
// 3. These devices are now paired.
// (4). Repeat 1 & 2 for any other Primos & Cubettos.
// (5). Power-off a Cubetto to un-pair (Primo is unaware of pairings, so no need to power-off).

// Uses AccelStepper Library
// (AccelStepper library must be installed).


// Ideally Cubetto would have a user-operated 'power' momentary push-button attached to an interrupt,
// along with an LED internal to the the push-button or adjacent to it.
// Operating the push-button would cause Cubetto to 'wake' for a timed period (eg. 1 hour),
// with 'awake' indicated by the LED.
// A timer expiring would cause Cubetto to sleep the Atmel uC (and nRF24l01),
// unless the 'power' button is pressed again


#include <AccelStepper.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"


// version number
#define PRIMO_VERSION 1

// sounds - move to sounds.h
#define PRIMO_NOTE_B0  31
#define PRIMO_NOTE_C1  33
#define PRIMO_NOTE_CS1 35
#define PRIMO_NOTE_D1  37
#define PRIMO_NOTE_DS1 39
#define PRIMO_NOTE_E1  41
#define PRIMO_NOTE_F1  44
#define PRIMO_NOTE_FS1 46
#define PRIMO_NOTE_G1  49
#define PRIMO_NOTE_GS1 52
#define PRIMO_NOTE_A1  55
#define PRIMO_NOTE_AS1 58
#define PRIMO_NOTE_B1  62
#define PRIMO_NOTE_C2  65
#define PRIMO_NOTE_CS2 69
#define PRIMO_NOTE_D2  73
#define PRIMO_NOTE_DS2 78
#define PRIMO_NOTE_E2  82
#define PRIMO_NOTE_F2  87
#define PRIMO_NOTE_FS2 93
#define PRIMO_NOTE_G2  98
#define PRIMO_NOTE_GS2 104
#define PRIMO_NOTE_A2  110
#define PRIMO_NOTE_AS2 117
#define PRIMO_NOTE_B2  123
#define PRIMO_NOTE_C3  131
#define PRIMO_NOTE_CS3 139
#define PRIMO_NOTE_D3  147
#define PRIMO_NOTE_DS3 156
#define PRIMO_NOTE_E3  165
#define PRIMO_NOTE_F3  175
#define PRIMO_NOTE_FS3 185
#define PRIMO_NOTE_G3  196
#define PRIMO_NOTE_GS3 208
#define PRIMO_NOTE_A3  220
#define PRIMO_NOTE_AS3 233
#define PRIMO_NOTE_B3  247
#define PRIMO_NOTE_C4  262
#define PRIMO_NOTE_CS4 277
#define PRIMO_NOTE_D4  294
#define PRIMO_NOTE_DS4 311
#define PRIMO_NOTE_E4  330
#define PRIMO_NOTE_F4  349
#define PRIMO_NOTE_FS4 370
#define PRIMO_NOTE_G4  392
#define PRIMO_NOTE_GS4 415
#define PRIMO_NOTE_A4  440
#define PRIMO_NOTE_AS4 466
#define PRIMO_NOTE_B4  494
#define PRIMO_NOTE_C5  523
#define PRIMO_NOTE_CS5 554
#define PRIMO_NOTE_D5  587
#define PRIMO_NOTE_DS5 622
#define PRIMO_NOTE_E5  659
#define PRIMO_NOTE_F5  698
#define PRIMO_NOTE_FS5 740
#define PRIMO_NOTE_G5  784
#define PRIMO_NOTE_GS5 831
#define PRIMO_NOTE_A5  880
#define PRIMO_NOTE_AS5 932
#define PRIMO_NOTE_B5  988
#define PRIMO_NOTE_C6  1047
#define PRIMO_NOTE_CS6 1109
#define PRIMO_NOTE_D6  1175
#define PRIMO_NOTE_DS6 1245
#define PRIMO_NOTE_E6  1319
#define PRIMO_NOTE_F6  1397
#define PRIMO_NOTE_FS6 1480
#define PRIMO_NOTE_G6  1568
#define PRIMO_NOTE_GS6 1661
#define PRIMO_NOTE_A6  1760
#define PRIMO_NOTE_AS6 1865
#define PRIMO_NOTE_B6  1976
#define PRIMO_NOTE_C7  2093
#define PRIMO_NOTE_CS7 2217
#define PRIMO_NOTE_D7  2349
#define PRIMO_NOTE_DS7 2489
#define PRIMO_NOTE_E7  2637
#define PRIMO_NOTE_F7  2794
#define PRIMO_NOTE_FS7 2960
#define PRIMO_NOTE_G7  3136
#define PRIMO_NOTE_GS7 3322
#define PRIMO_NOTE_A7  3520
#define PRIMO_NOTE_AS7 3729
#define PRIMO_NOTE_B7  3951
#define PRIMO_NOTE_C8  4186
#define PRIMO_NOTE_CS8 4435
#define PRIMO_NOTE_D8  4699
#define PRIMO_NOTE_DS8 4978


// Remove // comments from following line to enable debug tracing.
#define PRIMO_DEBUG_MODE 1

#ifdef PRIMO_DEBUG_MODE
#define debug_printf printf
#else
#define debug_printf(...) ((void) 0)
#endif


// The nRF24l01 can accept up to 32 bytes in a single radio packet,
// The comms protocol is designed to encapsulate a complete set of Primo movement commands
// in one packet along with an identifier for all Primo's, and a unique identifier for this Primo.
#define PRIMO_NRF24L01_MAX_PACKET_SIZE 32
static char packet[PRIMO_NRF24L01_MAX_PACKET_SIZE];

// This 32-bit value is the identifier for ANY 'Primo'.
// It is inserted into every radio packet sent by ANY Primo.
// A check is made on every radio packet, received by ANY Cubetto, that this value is present.
#define PRIMO_ID 0xDE1D8758 // NB This must be identical in Primo.

// This 32-bit value is randomly generated by a particular Primo every time it powers-up.
// It is inserted into every radio packet sent by that Primo.
// THIS Cubetto uses THIS value to pair to THAT Primo.
// The value stored here will be set to the value contained in the first valid packet received.
static long primo_random = 0;

// Primo can generate a streem of up to 16 movement commands.
// Each command is 1 of Forward, Left or Right.
// Stop has been assigned to 0x00 to cover any empty slots in a Primo.
// One byte has been used for each movement command.
// This allows extra space for future meta-data (e.g. position ordinal),
// or extra commands (e.g. BACK, 45TURN etc).
#define PRIMO_COMMAND_STOP    0x00
#define PRIMO_COMMAND_FORWARD 0x01
#define PRIMO_COMMAND_LEFT    0x02
#define PRIMO_COMMAND_RIGHT   0x03


#define PRIMO_STEPPER_MAX_SPEED    1000   // Pulses per second.
#define PRIMO_STEPPER_ACCELERATION 500    // Pulses per second.

#define PRIMO_STEPPER_FORWARD 2700   // Total pulses.
#define PRIMO_STEPPER_TURN    1170   // Total pulses.


//
// Hardware configuration
//

int in1Pin = 12;
int in2Pin = 10;
int in3Pin = 11;
int in4Pin = 9;

int on1Pin = 6;
int on2Pin = 4;
int on3Pin = 5;
int on4Pin = 3;

#define PRIMO_BUZZER_PIN 13

//int on1Pin = 6;
//int on2Pin = 5;
//int on3Pin = 4;
//int on4Pin = 3;


// Define a stepper and the pins it will use
AccelStepper stepper_left(AccelStepper::HALF4WIRE, in1Pin, in2Pin, in3Pin, in4Pin);
AccelStepper stepper_right(AccelStepper::HALF4WIRE, on1Pin, on2Pin, on3Pin, on4Pin);

static bool stepper_left_moving = 0;
static bool stepper_right_moving = 0;


// Set up nRF24L01 radio on SPI bus plus pins 7 (CE) & 8 (CSN).
RF24 radio(7, 8);


//
// Topology
//

// Flag to signal between interrupt handler and main event loop.
volatile bool rx_event_handler_pending = 0;

// Single radio pipe address for the 2 nodes to communicate.
const uint64_t pipe = 0xE8E8F0F0E1LL;

// Interrupt handler, check the radio because we got an IRQ
void check_radio(void);

// play a happy tune to indicate successful completion of a command sequence
void happy(void);

////////////////////////////////////////////////////////////////////////////////

void setup (void)
{
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);

  pinMode(on1Pin, OUTPUT);
  pinMode(on2Pin, OUTPUT);
  pinMode(on3Pin, OUTPUT);
  pinMode(on4Pin, OUTPUT);

  stepper_left.setMaxSpeed(PRIMO_STEPPER_MAX_SPEED);
  stepper_right.setMaxSpeed(PRIMO_STEPPER_MAX_SPEED);
  stepper_left.setAcceleration(PRIMO_STEPPER_ACCELERATION);
  stepper_right.setAcceleration(PRIMO_STEPPER_ACCELERATION);

  // IMPORTANT stepper outputs are disabled here to minimise power usage whilst stationary,
  // as they are automatically enabled in the class constructor.
  stepper_left.disableOutputs();
  stepper_right.disableOutputs();


  //
  // Print preamble
  //

  Serial.begin(57600);
  printf_begin();

  //while (Serial.read()== -1)
  debug_printf("ROLE: Cubetto - Version %d\n\r", PRIMO_VERSION);


  //
  // Setup and configure rf radio
  //

  radio.begin();

  // We will be using the Ack Payload feature, so please enable it
  radio.enableAckPayload();

  // Open pipes to other nodes for communication
  radio.openReadingPipe(1, pipe);

  // Start listening
  radio.startListening();

  // Dump the configuration of the rf unit for debugging
  radio.printDetails();

  // Attach interrupt handler to interrupt #0 (using pin 2)
  // on BOTH the sender and receiver
  attachInterrupt(1, check_radio, CHANGE);

  happy();
  happy();

  delay(100);
}

////////////////////////////////////////////////////////////////////////////////

static uint32_t ackMessageCount = 0;

////////////////////////////////////////////////////////////////////////////////

void loop (void)
{
  // Check if there is an Rx event to handle.
  if (rx_event_handler_pending == 1)
  {
    int i;
    long packet_id = 0;
    long packet_random = 0;

    debug_printf("Event Handler start\n\r");

    debug_printf("packet");
    for (i = 0; i < 32; i++)
    {
      debug_printf(" %x", packet[i]);
    }
    debug_printf("\n\r");

    memcpy(&packet_id, (const long*) &packet[0], 4);
    memcpy(&packet_random, (const long*) &packet[4], 4);

    // Check the packet is valid.
    if (packet_id == PRIMO_ID)
    {
      debug_printf("PRIMO_ID good\n\r");

      if (primo_random == 0)
      {
        // (The UID could be used to set the packet address in the radio, but this would
        // make it necessary to un-pair Primo/Cubetto at BOTH ends).
        primo_random = packet_random;
        debug_printf("PRIMO_RANDOM set\n\r");
      }

      if (primo_random == packet_random)
      {
        debug_printf("PRIMO_RANDOM good\n\r");

        // Carry out movement instructions here.
        // IMPORTANT stepper outputs are enabled/disabled to minimise power usage whilst stationary.
        stepper_left.enableOutputs();
        stepper_right.enableOutputs();
        move();
        stepper_left.disableOutputs();
        stepper_right.disableOutputs();
      }
    }

    debug_printf("Event Handler end\n\r");

    // Finally clear the event flag, so that the next interrupt event can register.
    rx_event_handler_pending = 0;

    // FIXME Would it be better to disable the radiointerrupt in the interrupt handler,
    // and re-enable it here?
  }
}

////////////////////////////////////////////////////////////////////////////////

void move (void)
{
  int i;

  for (i = 8; i < 32; i++)
  {
    switch (packet[i])
    {
      case PRIMO_COMMAND_STOP:
        happy();
        break;

      case PRIMO_COMMAND_FORWARD:
        stepper_left.move(-PRIMO_STEPPER_FORWARD);
        stepper_right.move(PRIMO_STEPPER_FORWARD);
        tone(PRIMO_BUZZER_PIN, 3000);
        delay(50);
        noTone(PRIMO_BUZZER_PIN);
        break;

      case PRIMO_COMMAND_LEFT:
        stepper_left.move(-PRIMO_STEPPER_TURN);
        stepper_right.move(-PRIMO_STEPPER_TURN);
        tone(PRIMO_BUZZER_PIN, 5000);
        delay(50);
        noTone(PRIMO_BUZZER_PIN);
        break;

      case PRIMO_COMMAND_RIGHT:
        stepper_left.move(PRIMO_STEPPER_TURN);
        stepper_right.move(PRIMO_STEPPER_TURN);
        tone(PRIMO_BUZZER_PIN, 4000);
        delay(50);
        noTone(PRIMO_BUZZER_PIN);
        break;

      default:
        break;
    }

    stepper_left_moving = 1;
    stepper_right_moving = 1;

    while (stepper_left_moving || stepper_right_moving)
    {
      stepper_left_moving = stepper_left.run();
      stepper_right_moving = stepper_right.run();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void check_radio (void)
{
  // What happened?
  bool tx, fail, rx;
  radio.whatHappened(tx, fail, rx);

  debug_printf("Got Interupt from the Radio\n\r");

  // Have we successfully transmitted?
  if (tx)
  {
    debug_printf("Ack Payload:Sent\n\r");
  }

  // Have we failed to transmit?
  if (fail)
  {
    debug_printf("Ack Payload:Failed\n\r");
  }

  // Did we receive a message?
  if (rx)
  {
    // Signal the main event loop that there is an Rx event to handle.
    rx_event_handler_pending = 1;

    // Get this payload and dump it
    radio.read(packet, PRIMO_NRF24L01_MAX_PACKET_SIZE);
    debug_printf("Got payload\n\r");

    // Add an ack packet for the next time around.  This is a simple
    // packet counter
    radio.writeAckPayload(1, &ackMessageCount, sizeof(ackMessageCount));
    ++ackMessageCount;
  }
}

////////////////////////////////////////////////////////////////////////////////

void happy()
{
  tone(PRIMO_BUZZER_PIN, PRIMO_NOTE_B0, 150);
  delay(20);

  tone(PRIMO_BUZZER_PIN, PRIMO_NOTE_E7, 600);
  delay(40);

  tone(PRIMO_BUZZER_PIN, PRIMO_NOTE_D3, 150);
  delay(20);

  //tone(PRIMO_BUZZER_PIN, PRIMO_NOTE_C8, 500);
  //delay(10);
}

