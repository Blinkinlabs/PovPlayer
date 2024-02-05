#ifndef ICND2110_H_
#define ICND2110_H_

extern void icnd2110_begin(size_t count, int spiClockFrequency);

extern void icnd2110_SendData(const unsigned char * data, size_t count, int spiClockFrequency);

#endif
