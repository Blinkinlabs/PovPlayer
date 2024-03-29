// MBI6020.h: Driver for controlling strings of MBI6020 pixel LEDs.
// 2018 Blinkinlabs, LLC

#ifndef MBI6020_H_
#define MBI6020_H_

//#include "LED_DATA.h"


extern void mbi6020_begin(size_t count, int spiClockFrequency);

extern void send16bitConfigurationData(size_t count, int spiClockFrequency);

extern void send8bitDotCorrectionDataHigh(size_t count, int spiClockFrequency);

extern void send16bitGrayscaleData(const uint8_t * data, size_t count, int spiClockFrequency);

#endif
