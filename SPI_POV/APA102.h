// APA102.h: Driver for controlling strings of MBI6020 pixel LEDs.
// 2018 Blinkinlabs, LLC

#ifndef APA102_H_
#define APA102_H_

#include "LED_DATA.h"

const int SPI_CLOCK_FREQ = 10000000;

extern void apa102_begin();

extern void apa102_SendData(const LED_Data * data, size_t count);

#endif