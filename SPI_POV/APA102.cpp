// APA102.cpp: Driver for controlling strings of APA102 pixel LEDs.
// 2018 Blinkinlabs, LLC

#include <SPI.h>
#include "APA102.h"

void apa102_begin(size_t count, int spiClockFrequency) {
  SPI1.begin();
}

void apa102_SendData(const uint8_t * data, size_t count, int spiClockFrequency) {
  uint8_t buff[4];
  buff[0] = 0;
  buff[1] = 0;
  buff[2] = 0;
  buff[3] = 0;
  
  SPI1.beginTransaction(SPISettings(spiClockFrequency, MSBFIRST, SPI_MODE0));

  // Start frame: 32 bits of 0's
  SPI1.transfer(buff, 4);
//  SPI1.transfer(0x00);
//  SPI1.transfer(0x00);
//  SPI1.transfer(0x00);
//  SPI1.transfer(0x00);

  // Data: 32-bits per LED
  for(size_t i = 0; i < count; i++) {
    // GBR
    SPI1.transfer(0xFF);
    SPI1.transfer(*(data + 2));
    SPI1.transfer(*(data + 1));
    SPI1.transfer(*(data + 0));

    data += 3;
  }
  
  // End of frame
  // Note: Must send ledCount/2 bits of 0's to clock everything out
  uint32_t zeroBytes = (((count+1)/2) + 7)/8;

  for (;zeroBytes > 0; zeroBytes--) {
    SPI1.transfer(0x00);
  }  

  SPI1.endTransaction();
}
