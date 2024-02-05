#include <Arduino.h>
#include "MBI6120.h"

#include <SPI.h>

// TODO: Make me configurable?
// TODO: Replace driver on board, switch to data output
#define dataPin 32  

void MBI6120::begin() {
  //TODO: Pin config
//  SPI1.stop();
  pinMode(dataPin, OUTPUT);
  digitalWrite(dataPin, LOW);
}


void MBI6120::send(const uint8_t * data, size_t count) {
/*
# Packet structure

[Header][ICn data][ICn-1 data]...[Ic1 data]

## Header

[H1][CLK][CMD][NUM][gap]

| Section  | Bits    | Value                               | Function            |
| ===      | ===     | ===                                 | ===                 |
| H1       | [35:24] | 12’b111111111111                    | Data rate detection |
| reserved | [23:22] | xx (don’t care)                     |                     |
| CLK      | [21:20] | 0=5.2MHz,1=2.6MHz,2=1,3MHz,3=650kHz | GCLK setting        |
| CMD      | [19:12] | 8’b00000000=data, 8’b01010101=reset | Packet type         |
| reserved | [11:10] | xx (don’t care)                     |                     |
| NUM      | [9:0]   | 1-511                               | Number of ICs       |

The gap is a stretched clock bit.

## Data

Data is 12-bit values for each output, MSB first. OUTA is sent first, then OUTB, then OUTC.

*/

  // H1
  for(int i = 0; i < 12; i++) {
    // Let's start with 4us per bit
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(3);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }
  
  // Reserved
  for(int i = 0; i < 2; i++) {
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(3);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }

  // CLK
  for(int i = 0; i < 2; i++) {
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(3);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }

  // CMD
  for(int i = 0; i < 8; i++) {
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(3);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }

  // Reserved
  for(int i = 0; i < 2; i++) {
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(3);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }

  // NUM
  for(int i = 0; i < 10; i++) {
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(dataPin, (count >> (9-i)) & 0x01);
    delayMicroseconds(3);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }

  // gap
  digitalWriteFast(dataPin, HIGH);
  delayMicroseconds(9);
  digitalWriteFast(dataPin, LOW);
  delayMicroseconds(1);

  //data
  for(int led = 0; led < count; led++) {
    for(int channel = 0; channel < 3; channel++) {
      int val = (data[led*3 + channel]) << 4;
      
      for(int index = 0; index < 12; index++) {
        digitalWriteFast(dataPin, HIGH);
        delayMicroseconds(1);
        
        digitalWriteFast(dataPin, (val >> (11-index)) & 0x01);
        delayMicroseconds(3);
        digitalWriteFast(dataPin, LOW);
        delayMicroseconds(1);
      }
    }

    // gap
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(9);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }
}


void MBI6120::send(const uint16_t * data, size_t count) {
/*
# Packet structure

[Header][ICn data][ICn-1 data]...[Ic1 data]

## Header

[H1][CLK][CMD][NUM][gap]

| Section  | Bits    | Value                               | Function            |
| ===      | ===     | ===                                 | ===                 |
| H1       | [35:24] | 12’b111111111111                    | Data rate detection |
| reserved | [23:22] | xx (don’t care)                     |                     |
| CLK      | [21:20] | 0=5.2MHz,1=2.6MHz,2=1,3MHz,3=650kHz | GCLK setting        |
| CMD      | [19:12] | 8’b00000000=data, 8’b01010101=reset | Packet type         |
| reserved | [11:10] | xx (don’t care)                     |                     |
| NUM      | [9:0]   | 1-511                               | Number of ICs       |

The gap is a stretched clock bit.

## Data

Data is 12-bit values for each output, MSB first. OUTA is sent first, then OUTB, then OUTC.

*/

  // H1
  for(int i = 0; i < 12; i++) {
    // Let's start with 4us per bit
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(3);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }
  
  // Reserved
  for(int i = 0; i < 2; i++) {
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(3);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }

  // CLK
  for(int i = 0; i < 2; i++) {
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(3);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }

  // CMD
  for(int i = 0; i < 8; i++) {
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(3);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }

  // Reserved
  for(int i = 0; i < 2; i++) {
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(3);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }

  // NUM
  for(int i = 0; i < 10; i++) {
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(dataPin, (count >> (9-i)) & 0x01);
    delayMicroseconds(3);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }

  // gap
  digitalWriteFast(dataPin, HIGH);
  delayMicroseconds(9);
  digitalWriteFast(dataPin, LOW);
  delayMicroseconds(1);

  //data
  for(int led = 0; led < count; led++) {
    for(int channel = 0; channel < 3; channel++) {
      int val = (data[led*3 + channel]) >> 4;
      
      for(int index = 0; index < 12; index++) {
        digitalWriteFast(dataPin, HIGH);
        delayMicroseconds(1);
        
        digitalWriteFast(dataPin, (val >> (11-index)) & 0x01);
        delayMicroseconds(3);
        digitalWriteFast(dataPin, LOW);
        delayMicroseconds(1);
      }
    }

    // gap
    digitalWriteFast(dataPin, HIGH);
    delayMicroseconds(9);
    digitalWriteFast(dataPin, LOW);
    delayMicroseconds(1);
  }
}
