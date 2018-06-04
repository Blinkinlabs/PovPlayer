// APA102.cpp: Driver for controlling strings of APA102 pixel LEDs.
// 2018 Blinkinlabs, LLC

#include <SPI.h>
#include "APA102.h"

void apa102_begin(int spiClockFrequency) {
  SPI1.begin();
}

//void apa102_SendData(const LED_Data * data, size_t count, int spiClockFrequency) {
//  SPI1.beginTransaction(SPISettings(spiClockFrequency, MSBFIRST, SPI_MODE0));
//
//  // Start frame: 32 bits of 0's
//  SPI1.transfer(0x00);
//  SPI1.transfer(0x00);
//  SPI1.transfer(0x00);
//  SPI1.transfer(0x00);
//
//  // Data: 32-bits per LED
//  for(size_t i = 0; i < count; i++) {
//    SPI1.transfer(0xFF);
//    SPI1.transfer(data[i].b);
//    SPI1.transfer(data[i].g);
//    SPI1.transfer(data[i].r);
//  }
//  
//  // End of frame
//  // Note: Must send ledCount/2 bits of 0's to clock everything out
//  uint32_t zeroBytes = (((count+1)/2) + 7)/8;
//
//  for (;zeroBytes > 0; zeroBytes--) {
//    SPI1.transfer(0x00);
//  }  
//
//  SPI1.endTransaction();
//}

void apa102_SendData(const unsigned char * data, size_t count, int spiClockFrequency) {
  SPI1.beginTransaction(SPISettings(spiClockFrequency, MSBFIRST, SPI_MODE0));

  // Start frame: 32 bits of 0's
  SPI1.transfer(0x00);
  SPI1.transfer(0x00);
  SPI1.transfer(0x00);
  SPI1.transfer(0x00);

  // Data: 32-bits per LED
  for(size_t i = 0; i < count; i++) {
    
    SPI1.transfer(0xFF);
    SPI1.transfer(*(data++));
    SPI1.transfer(*(data++));
    SPI1.transfer(*(data++));
  }
  
  // End of frame
  // Note: Must send ledCount/2 bits of 0's to clock everything out
  uint32_t zeroBytes = (((count+1)/2) + 7)/8;

  for (;zeroBytes > 0; zeroBytes--) {
    SPI1.transfer(0x00);
  }  

  SPI1.endTransaction();
}
