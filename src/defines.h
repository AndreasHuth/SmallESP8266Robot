/*
 * defines.h
 *
 *  Created on: 25.09.2017
 *      Author: anonymous
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#define PIN_MOTOR2_PWM 14        //D2
#define PIN_MOTOR2_DIR 2        

#define PIN_MOTOR1_PWM 13       
#define PIN_MOTOR1_DIR 12       

#define PIN_BUZZER 15       


#define HARD_RESET_PIN  0        // D3

#define CONTROLINTERVAL 120

#define MAX_ACCEL       40

#define MOTOROFFSET     200
#define MAXSPEED        1020

#define STOPDISTANCE    9

// Telemetry
#define TELEMETRY_BATTERY 1
#define TELEMETRY_ANGLE 1
//#define TELEMETRY_DEBUG 1  // Dont use TELEMETRY_ANGLE and TELEMETRY_DEBUG at the same time!

//#define DEBUG
#define DEBUG1
#define DEBUG2

#define PRINTLN(var) Serial.print(#var ": "); Serial.println(var)

#endif /* DEFINES_H_ */
