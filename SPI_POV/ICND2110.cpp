#include <SPI.h>
#include <DmaSpi.h>

#include "ICND2110.h"

// Max 
#define DMA_MAXLEDS   512

// 7 bytes per LED (16*3+1 blank overhead), plus 16 start, 2 blank, 2 register, 2 blank, 19 stop
#define DMA_MAXSIZE   (DMA_MAXLEDS*7 + 41)

// Source DMA buffer
uint8_t srcBuffer[DMA_MAXSIZE];
uint8_t currentSize;

// Lookup table for 8->16 conversion
uint8_t lookup_table_h[256];
uint8_t lookup_table_l[256];

void icnd2110_begin(size_t count, int spiClockFrequency) {
  SPI1.begin();

  const float gamma = 1.8;
  const float brightness = .55;

  for(int i = 0; i < 256; i++) {
    const uint16_t val = 65535*pow(brightness*i/255.0,gamma);
    lookup_table_h[i] = (val >> 8) & 0xFF;
    lookup_table_l[i] = (val     ) & 0xFF;
    
//    Serial.print(i);
//    Serial.print(":");
//    Serial.println(val);
  }
}

void send16bits(unsigned char data) {
//  const uint16_t d =lookup_table[data];
  SPI1.transfer(lookup_table_h[data]);
  SPI1.transfer(lookup_table_l[data]);

//  Serial.print(data);
//  Serial.print(", ");
//  Serial.print((d >> 8) & 0xFF);
//  Serial.println((d     ) & 0xFF);
}

void icnd2110_SendData(const unsigned char * data, size_t count, int spiClockFrequency) {
  int pos = 0;   // offset into the DMA buffer
  
  // The driver has 4 outputs, so count needs to be aligned to that.
  count -= count%4;
  
//  SPI1.beginTransaction(SPISettings(spiClockFrequency, MSBFIRST, SPI_MODE0));

//  // Start: 128 bits of 1
//  for(int i = 0; i < 16; i++)
//    SPI1.transfer(0xFF);
  for(int i = 0; i < 16; i++)
    srcBuffer[pos++] = 0xFF;

  // Blank: 16 bits of 0
//  SPI1.transfer(0x00);
//  SPI1.transfer(0x00);
  srcBuffer[pos++] = 0x00;
  srcBuffer[pos++] = 0x00;

  // Reg: 0x0018: PWM-wider, UP
//  SPI1.transfer(0x00);
//  SPI1.transfer(0x18);
  srcBuffer[pos++] = 0x00;
  srcBuffer[pos++] = 0x18;

  // Blank: 16 bits of 0
//  SPI1.transfer(0x00);
//  SPI1.transfer(0x00);
  srcBuffer[pos++] = 0x00;
  srcBuffer[pos++] = 0x00;

  // Data: 32-bits per LED
  for(size_t i = 0; i < count; i+=4) {
    // OUT5-OUT0 (96 bits)
    srcBuffer[pos++] = lookup_table_h[data[1*3 + 2]];        // OUT5 = B2
    srcBuffer[pos++] = lookup_table_l[data[1*3 + 2]];
    srcBuffer[pos++] = lookup_table_h[data[1*3 + 1]];        // OUT4 = G2
    srcBuffer[pos++] = lookup_table_l[data[1*3 + 1]];
    srcBuffer[pos++] = lookup_table_h[data[1*3 + 0]];        // OUT3 = R2
    srcBuffer[pos++] = lookup_table_l[data[1*3 + 0]];
    
    srcBuffer[pos++] = lookup_table_h[data[0*3 + 2]];        // OUT2 = B1
    srcBuffer[pos++] = lookup_table_l[data[0*3 + 2]];
    srcBuffer[pos++] = lookup_table_h[data[0*3 + 1]];        // OUT1 = G1
    srcBuffer[pos++] = lookup_table_l[data[0*3 + 1]];
    srcBuffer[pos++] = lookup_table_h[data[0*3 + 0]];        // OUT0 = R1
    srcBuffer[pos++] = lookup_table_l[data[0*3 + 0]];
    
    // Blank: 16 bits of 0
//    SPI1.transfer(0x00);
//    SPI1.transfer(0x00);
    srcBuffer[pos++] = 0x00;
    srcBuffer[pos++] = 0x00;

    // OUT11-OUT6 (96 bits)
    srcBuffer[pos++] = lookup_table_h[data[3*3 + 2]];        // OUT11 = B4
    srcBuffer[pos++] = lookup_table_l[data[3*3 + 2]];
    srcBuffer[pos++] = lookup_table_h[data[3*3 + 1]];        // OUT10 = G4
    srcBuffer[pos++] = lookup_table_l[data[3*3 + 1]];
    srcBuffer[pos++] = lookup_table_h[data[3*3 + 0]];        // OUT9 = R4
    srcBuffer[pos++] = lookup_table_l[data[3*3 + 0]];
    
    srcBuffer[pos++] = lookup_table_h[data[2*3 + 2]];        // OUT8 = B3
    srcBuffer[pos++] = lookup_table_l[data[2*3 + 2]];
    srcBuffer[pos++] = lookup_table_h[data[2*3 + 1]];        // OUT7 = G3
    srcBuffer[pos++] = lookup_table_l[data[2*3 + 1]];
    srcBuffer[pos++] = lookup_table_h[data[2*3 + 0]];        // OUT6 = R3
    srcBuffer[pos++] = lookup_table_l[data[2*3 + 0]];

    // Blank: 16 bits of 0
//    SPI1.transfer(0x00);
//    SPI1.transfer(0x00);
    srcBuffer[pos++] = 0x00;
    srcBuffer[pos++] = 0x00;
    
    data += 12; // 4*RGB = 12 bytes per loop
  }
  
  // End: 145 (?) bits of 1
//  for(int i = 0; i < 18; i++)
//    SPI1.transfer(0xFF);
  for(int i = 0; i < 18; i++)
    srcBuffer[pos++] = 0xFF;

  // One extra bit (it /should/ only be a bit, but that's not possible with DMA
//  SPI1.transfer(0x80);
//  srcBuffer[pos++] = 0x80;
  srcBuffer[pos++] = 0x80;

//  SPI1.beginTransaction(SPISettings(spiClockFrequency, MSBFIRST, SPI_MODE0));
//  for(int i = 0; i < pos; i++)
//    SPI1.transfer(srcBuffer[i]);
//  SPI1.endTransaction();



  DMASPI1.begin();
  DMASPI1.setSettings(SPISettings(spiClockFrequency, MSBFIRST, SPI_MODE0));
  DMASPI1.start();
//  SPI1.beginTransaction(SPISettings(spiClockFrequency, MSBFIRST, SPI_MODE0));

  DmaSpi::Transfer trx(srcBuffer, pos, nullptr);
  DMASPI1.registerTransfer(trx);
  while(trx.busy())
  {
  }

//  SPI1.endTransaction();
}

