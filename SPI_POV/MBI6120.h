// MBI6120.h: Driver for controlling strings of MBI6120 pixel LEDs.
// 2019 Blinkinlabs, LLC

#pragma once

//#include "LED_DATA.h"

class MBI6120 {
public:
  void begin();
  // TODO: CLK selection

  // Send a reset packet
  //void reset();

  void send(const uint8_t * data, size_t count);
  void send(const uint16_t * data, size_t count);
};
