// MBI6020.h: Driver for controlling strings of MBI6020 pixel LEDs.
// 2018 Blinkinlabs, LLC

#ifndef MBI6020_H_
#define MBI6020_H_

#include "LED_DATA.h"


extern void mbi6020_begin();

extern void send16bitConfigurationData(size_t count);
extern void send8bitDotCorrectionDataHigh(size_t count);
extern void send16bitGrayscaleData(const LED_Data * data, size_t count);

#endif
