#include <Arduino.h>
#include "defines.h"
#include "globals.h"
#include "Wire.h"
#include <VL53L0X.h>

VL53L0X sensor;

void VL53L0Xsetup(void)
{
  Wire.begin();
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1) {}
  }
  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
  sensor.startContinuous();
}

uint16_t VL53L0Xloop(void)
{
   uint16_t value; 

  value = sensor.readRangeContinuousMillimeters();

  #ifdef DEBUG
    Serial.print(value);
    if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

    Serial.println();
  #endif
  
  return value;
}