#include <Arduino.h>
#include "definitions.h"

#ifdef LED
void blink_led(int num_times, int blink_duration) {
  for (int i = 0; i < num_times; i++) {
    digitalWrite(LED, HIGH);
    delay(blink_duration / 2);
    digitalWrite(LED, LOW);
    delay(blink_duration / 2);
  }
}
#endif
