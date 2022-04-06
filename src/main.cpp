#include "defines.h"
#include "lib.h"
#include "motors.h"
#include "globals.h"
#include "wifiRC.h"
#include "VL53lib.h"
unsigned long currentMillis = millis();
unsigned long previousMillis = 0;
unsigned long updateMillis = 0;

const long interval = 1000; //interval for checking failsave state

bool obstacle = false;


//#########################################################################################################

void setup() {
    initMotor();
    initBuzzer();
    VL53L0Xsetup();
    initRC ();
    delay (200);
    buzzerOn200ms();
}

//#########################################################################################################

void loop(void) {

  ResetToFactory();
  webSocket.loop();

  currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
#ifdef DEBUG
    Serial.printf("S1_6_flag=%d\n", S1_6_flag);
#endif
    previousMillis = currentMillis;
    if (S1_6_flag == false) {
      SameStateCount++;
      if (SameStateCount >= FailSaveTime) {
        stopMotor();
        //digitalWrite(2, LOW);
      }
    } else {
      //digitalWrite(2, HIGH);
      SameStateCount = 0;
      S1_6_flag = false;
    }

    VL53L0Xloop();
    getLipoVoltage(); 
    
  }
  if (SameStateCount < FailSaveTime) {
    if (connected){
      if (currentMillis - updateMillis >= CONTROLINTERVAL) {
        updateMillis = currentMillis;
        if (VL53L0Xloop() < 150) {
          obstacle = true;
        } else {
          obstacle = false;
        }
          CalcSetMotorSpeed(obstacle);
      }
    }
    else{
      stopMotor();
    }
  }
  server.handleClient();
}
