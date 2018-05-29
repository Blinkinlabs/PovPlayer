// MBI6020.cpp: Driver for controlling strings of MBI6020 pixel LEDs.
// 2018 Blinkinlabs, LLC

#include <SPI.h>
#include "MBI6020.h"

#define HEADER_16B_GRAYSCALE_DATA (0x3F)
#define HEADER_10B_GRAYSCALE_DATA (0x2B)
#define HEADER_8B_DOT_CORRECTION_DATA (0x33)
#define HEADER_6B_DOT_CORRECTION_DATA (0x27)
#define HEADER_16B_CONFIGURATION_DATA (0x23)
#define HEADER_10B_CONFIGURATION_DATA (0x37)


const int SPI_CLOCK_FREQ = 10000000;

bool computeParity(size_t data) {
  bool parity = false;
  
  for(uint32_t b = 0; b < sizeof(size_t)*8; b++)
    if(data & (1 << b))
      parity = !parity;

  return parity;
}

void send16bitConfigurationData(size_t count) {
  // Build the preamble
  const size_t countAdj = count;

  uint8_t parity = 0;
  if(computeParity(countAdj))
    parity |= (1 << 0);
  if(computeParity(0))
    parity |= (1 << 1);
  if(computeParity(HEADER_16B_CONFIGURATION_DATA))
    parity |= (1 << 2);
  if(computeParity(parity))  
    parity |= (1 << 3);

  uint16_t preamble[3];
  preamble[0] = (HEADER_16B_CONFIGURATION_DATA << 10);
  preamble[1] = (HEADER_16B_CONFIGURATION_DATA << 10) | (countAdj & 0x3FF);
  preamble[2] = ((parity & 0x0F) << 12) | (countAdj & 0x3FF);

  // Default is: 1001111110 (0x27E)
//  const uint16_t CF1 = 0x027E;
  const uint16_t CF1 = (0x3 << 8) | (0x3 << 5) | (1 << 3) | (0x3 << 1);
  const uint8_t CF2 = 0x07;

  uint16_t configData[3];
  configData[0] = (CF1 & 0x3FF);
  configData[1] = (CF1 & 0x3FF);
  configData[2] = (CF2 & 0x07);

  SPI.beginTransaction(SPISettings(SPI_CLOCK_FREQ, MSBFIRST, SPI_MODE1));

  // send the preamble
  for(uint8_t i = 0; i < 3; i++) {
    SPI.transfer((preamble[i] >> 8 ) & 0xFF);
    SPI.transfer((preamble[i]      ) & 0xFF);
  }

  // send the config
  for(uint8_t i = 0; i < count; i++) {
    SPI.transfer((configData[0] >> 8 ) & 0xFF);
    SPI.transfer((configData[0]      ) & 0xFF);
    SPI.transfer((configData[1] >> 8 ) & 0xFF);
    SPI.transfer((configData[1]      ) & 0xFF);
    SPI.transfer((configData[2] >> 8 ) & 0xFF);
    SPI.transfer((configData[2]      ) & 0xFF);
  }

  SPI.endTransaction();
}

// Send correction data, except that we don't care about sending correction data
void send8bitDotCorrectionDataHigh(size_t count) {
  // Build the preamble
  const size_t countAdj = count - 1;

  uint8_t parity = 0;
  if(computeParity(countAdj))
    parity |= (1 << 0);
  if(computeParity(0))
    parity |= (1 << 1);
  if(computeParity(HEADER_8B_DOT_CORRECTION_DATA))
    parity |= (1 << 2);
  if(computeParity(parity))  
    parity |= (1 << 3);

  uint16_t preamble[3];
  preamble[0] = (HEADER_8B_DOT_CORRECTION_DATA << 10);
  preamble[1] = (HEADER_8B_DOT_CORRECTION_DATA << 10) | (countAdj & 0x3FF);
  preamble[2] = ((parity & 0x0F) << 12) | (countAdj & 0x3FF);

  SPI.beginTransaction(SPISettings(SPI_CLOCK_FREQ, MSBFIRST, SPI_MODE1));

  // send the preamble
  for(uint8_t i = 0; i < 3; i++) {
    SPI.transfer((preamble[i] >> 8 ) & 0xFF);
    SPI.transfer((preamble[i]      ) & 0xFF);
  }

  // send the LED data
  for(uint8_t i = 0; i < count; i++) {
    SPI.transfer(0xFF);
    SPI.transfer(0xFF);
    SPI.transfer(0xFF);
    SPI.transfer(0xFF);
    SPI.transfer(0xFF);
    SPI.transfer(0xFF);
  }

  SPI.endTransaction();
}

void send16bitGrayscaleData(const LED_Data * data, size_t count) {
  // Build the preamble
  const size_t countAdj = count - 1;

  uint8_t parity = 0;
  if(computeParity(countAdj))
    parity |= (1 << 0);
  if(computeParity(0))
    parity |= (1 << 1);
  if(computeParity(HEADER_16B_GRAYSCALE_DATA))
    parity |= (1 << 2);
  if(computeParity(parity))  
    parity |= (1 << 3);

  uint16_t preamble[3];
  preamble[0] = (HEADER_16B_GRAYSCALE_DATA << 10);
  preamble[1] = (HEADER_16B_GRAYSCALE_DATA << 10) | (countAdj & 0x3FF);
  preamble[2] = ((parity & 0x0F) << 12) | (countAdj & 0x3FF);

  SPI.beginTransaction(SPISettings(SPI_CLOCK_FREQ, MSBFIRST, SPI_MODE1));

  // send the preamble
  for(uint8_t i = 0; i < 3; i++) {
    SPI.transfer((preamble[i] >> 8 ) & 0xFF);
    SPI.transfer((preamble[i]      ) & 0xFF);
  }

  // send the LED data
  for(uint8_t i = 0; i < count; i++) {
    SPI.transfer((data[i].r >> 8 ) & 0xFF);
    SPI.transfer((data[i].r      ) & 0xFF);
    SPI.transfer((data[i].g >> 8 ) & 0xFF);
    SPI.transfer((data[i].g      ) & 0xFF);
    SPI.transfer((data[i].b >> 8 ) & 0xFF);
    SPI.transfer((data[i].b      ) & 0xFF);
  }

  SPI.endTransaction();

}
