#pragma once

#include "esp32-hal-log.h"

#include <RF24.h>

class Radio {
private:
  RF24 rf;

public:
  Radio(int cePin, int csnPin);

  bool initialize();
  RF24 *getDriver();
};