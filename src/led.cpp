#include "led.h"

namespace Led {

void init() {
  pinMode(LED_FLASH, OUTPUT);
  pinMode(LED_STATUS, OUTPUT);
  digitalWrite(LED_FLASH, LOW);
  digitalWrite(LED_STATUS, HIGH);  // off (active low)
}

void success() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_STATUS, LOW);
    delay(300);
    digitalWrite(LED_STATUS, HIGH);
    delay(300);
  }
}

void error() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_STATUS, LOW);
    delay(100);
    digitalWrite(LED_STATUS, HIGH);
    delay(100);
  }
}

void working() {
  digitalWrite(LED_STATUS, LOW);
}

void idle() {
  digitalWrite(LED_STATUS, HIGH);
}

void flash(bool on) {
  digitalWrite(LED_FLASH, on ? HIGH : LOW);
}

// Blink LED n times to indicate bin number (1-6)
void showBinNumber(int n) {
  if (n < 1 || n > 6) return;

  // Short pause before showing bin number
  delay(500);

  for (int i = 0; i < n; i++) {
    digitalWrite(LED_STATUS, LOW);   // ON
    delay(400);
    digitalWrite(LED_STATUS, HIGH);  // OFF
    delay(400);
  }
}

}
