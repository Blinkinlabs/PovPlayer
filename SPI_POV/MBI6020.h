// MBI6020.h: Driver for controlling strings of MBI6020 pixel LEDs.
// 2018 Blinkinlabs, LLC

#ifndef MBI6020_H_
#define MBI6020_H_


extern void mbi6020_begin(size_t count, int spiClockFrequency);

extern void send16bitConfigurationData(size_t count, int spiClockFrequency);

extern void send8bitDotCorrectionDataHigh(size_t count, int spiClockFrequency);

// Send data. Note that the input data is 8-bit, though the output will be 16 bit.
extern void send16bitGrayscaleData(const unsigned char * data, size_t count, int spiClockFrequency);

#endif
