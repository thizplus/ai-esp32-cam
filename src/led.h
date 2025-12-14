#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include "config.h"

namespace Led {
  void init();
  void success();           // 2x slow blink
  void error();             // 5x fast blink
  void working();           // LED on
  void idle();              // LED off
  void flash(bool on);
  void showBinNumber(int n); // Blink n times to show bin number (1-6)
}

#endif
