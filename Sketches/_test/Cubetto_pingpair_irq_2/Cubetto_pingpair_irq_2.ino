/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 Update 2014 - TMRh20
 */

/**
 * Example of using interrupts
 *
 * This is an example of how to user interrupts to interact with the radio, and a demonstration
 * of how to use them to sleep when receiving, and not miss any payloads. 
 * The pingpair_sleepy example expands on sleep functionality with a timed sleep option for the transmitter.
 * Sleep functionality is built directly into my fork of the RF24Network library
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

// Hardware configuration
RF24 radio(7,8);                          // Set up nRF24L01 radio on SPI bus plus pins 7 & 8
                                        
// Demonstrates another method of setting up the addresses
byte address[][5] = { 0xCC,0xCE,0xCC,0xCE,0xCC , 0xCE,0xCC,0xCE,0xCC,0xCE};

static uint32_t message_count = 0;


/********************** Setup *********************/

void setup(){

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);
  printf_begin();
  printf("\n\rRF24/examples/pingpair_irq/\n\r");
  printf("ROLE: Cubetto\n\r");

  // Setup and configure rf radio
  radio.begin();  
  //radio.setPALevel(RF24_PA_LOW);
  radio.enableAckPayload();                         // We will be using the Ack Payload feature, so please enable it
  radio.enableDynamicPayloads();                    // Ack payloads are dynamic payloads

  // Open pipes to other node for communication
  radio.openWritingPipe(address[1]);
  radio.openReadingPipe(1,address[0]);
  radio.startListening();
  radio.writeAckPayload( 1, &message_count, sizeof(message_count) );  // Add an ack packet for the next time around.  This is a simple
  ++message_count;        

  radio.printDetails();                             // Dump the configuration of the rf unit for debugging
  delay(50);
  attachInterrupt(1, check_radio, LOW);             // Attach interrupt handler to interrupt #1 (using pin 2 on Arduino Leonardo)
}



/********************** Main Loop *********************/
void loop() {

  // Receiver does nothing except in IRQ
  
}


/********************** Interrupt *********************/

void check_radio(void)                                // Receiver role: Does nothing!  All the work is in IRQ
{
  
  bool tx,fail,rx;
  radio.whatHappened(tx,fail,rx);                     // What happened?
  
  if ( tx ) {                                         // Have we successfully transmitted?
      printf("Ack Payload:Sent\n\r");
  }
  
  if ( fail ) {                                       // Have we failed to transmit?
      printf("Ack Payload:Failed\n\r");
  }
  
  if ( rx || radio.available()){                      // Did we receive a message?
    
    // We've received a time message
    digitalWrite(LED_BUILTIN, HIGH);
    static unsigned long got_time;                  // Get this payload and dump it
    radio.read( &got_time, sizeof(got_time) );
    printf("Got payload %lu\n\r",got_time);
    radio.writeAckPayload( 1, &message_count, sizeof(message_count) );  // Add an ack packet for the next time around.  This is a simple
    ++message_count;                                // packet counter
    digitalWrite(LED_BUILTIN, LOW);
  }
}
