/*
 * Motors.cpp
 *
 *  Created on: 25.09.2017
 *      Author: anonymous
 */

#include "defines.h"
#include "globals.h"


// ########################################################################################

void stopMotor(void)
{
    analogWriteRange(1023); 
    digitalWrite(PIN_MOTOR1_DIR, LOW);
    digitalWrite(PIN_MOTOR1_PWM, LOW);
    digitalWrite(PIN_MOTOR2_DIR, LOW);
    digitalWrite(PIN_MOTOR2_PWM, LOW);
}

// ########################################################################################

void initMotor(void)
{
    
    pinMode(PIN_MOTOR1_DIR, OUTPUT);
    pinMode(PIN_MOTOR1_PWM, OUTPUT);
    pinMode(PIN_MOTOR2_DIR, OUTPUT);
    pinMode(PIN_MOTOR2_PWM, OUTPUT);

    stopMotor();
}

// ########################################################################################

const byte controllerFA = 10; //PWM FORWARD PIN for OSMC Controller A (left motor)
const byte controllerRA = 9;  //PWM REVERSE PIN for OSMC Controller A (left motor)

const byte controllerFB = 6;  //PWM FORWARD PIN for OSMC Controller B (right motor)
const byte controllerRB = 5;  //PWM REVERSE PIN for OSMC Controller B (right motor)

int throttle, direction = 0; //throttle (Y axis) and direction (X axis) 

int leftMotor,leftMotorScaled = 0; //left Motor helper variables
float leftMotorScale = 0;

int rightMotor,rightMotorScaled = 0; //right Motor helper variables
float rightMotorScale = 0;

float maxMotorScale = 0; //holds the mixed output scaling factor

int deadZone = 10; //jostick dead zone 

void CalcSetMotorSpeed (bool disappear)
{
    // Wertebereich 0...180
    direction = (int) strtol(S1_buff, NULL, 16);
    throttle = (int) strtol(S2_buff, NULL, 16);

    // Wertebereich -90...+90
    direction -= 90;
    throttle -= 90;

    // Wertebereich  auf -1000 ... +1000 erweitern 
    direction *= 100;    direction /=   9;
    throttle *= 100;    throttle /=   9;

    //mix throttle and direction
    leftMotor = throttle+direction;
    rightMotor = throttle-direction;

    //print the initial mix results
    Serial.print("LIN:"); Serial.print( leftMotor, DEC);
    Serial.print(", RIN:"); Serial.print( rightMotor, DEC);

    //calculate the scale of the results in comparision base 8 bit PWM resolution
    leftMotorScale =  leftMotor/1000.0;
    leftMotorScale = abs(leftMotorScale);
    rightMotorScale =  rightMotor/1000.0;
    rightMotorScale = abs(rightMotorScale);

    Serial.print("| LSCALE:"); Serial.print( leftMotorScale,2);
    Serial.print(", RSCALE:"); Serial.print( rightMotorScale,2);

    //choose the max scale value if it is above 1
    maxMotorScale = max(leftMotorScale,rightMotorScale);
    maxMotorScale = max((float)1.0,maxMotorScale);

    //and apply it to the mixed values
    leftMotorScaled = constrain(leftMotor/maxMotorScale,-1000,1000);
    rightMotorScaled = constrain(rightMotor/maxMotorScale,-1000,1000);

    Serial.print("| LOUT:"); Serial.print( leftMotorScaled);
    Serial.print(", ROUT:"); Serial.print( rightMotorScaled);

    Serial.print(" |");

    //apply the results to appropriate uC PWM outputs for the LEFT motor:
    if(abs(leftMotorScaled)>deadZone)
    {
        if (leftMotorScaled > 0)
        {
            Serial.print("F");
            Serial.print(abs(leftMotorScaled),DEC);
    
            if (!disappear){
                digitalWrite(PIN_MOTOR1_DIR, LOW);
                analogWrite(PIN_MOTOR1_PWM, abs(leftMotorScaled));       
            }
            else
            {
                digitalWrite(PIN_MOTOR1_DIR, LOW);
                digitalWrite(PIN_MOTOR1_PWM, LOW);
            }

        }
        else 
        {
        Serial.print("R");
        Serial.print(abs(leftMotorScaled),DEC);

        digitalWrite(PIN_MOTOR1_DIR, HIGH);
        analogWrite(PIN_MOTOR1_PWM, 1000-abs(leftMotorScaled));
        }
    }  
    else 
    {
    Serial.print("IDLE");
    digitalWrite(PIN_MOTOR1_DIR, LOW);
    digitalWrite(PIN_MOTOR1_PWM, LOW);
    } 

    //apply the results to appropriate uC PWM outputs for the RIGHT motor:  
    if(abs(rightMotorScaled)>deadZone)
    {

        if (rightMotorScaled > 0)
        {
            Serial.print("F");
            Serial.print(abs(rightMotorScaled),DEC);

            if (!disappear){
                digitalWrite(PIN_MOTOR2_DIR, LOW);
                analogWrite(PIN_MOTOR2_PWM, abs(rightMotorScaled));
            }
            else
            {
                digitalWrite(PIN_MOTOR2_DIR, LOW);
                digitalWrite(PIN_MOTOR2_PWM, LOW);    
            }
        }
        else 
        {
            Serial.print("R");
            Serial.print(abs(rightMotorScaled),DEC);

            digitalWrite(PIN_MOTOR2_DIR, HIGH);
            analogWrite(PIN_MOTOR2_PWM, 1024-abs(rightMotorScaled));
        }
    }  
    else 
    {
    Serial.print("IDLE");
    digitalWrite(PIN_MOTOR2_DIR, LOW);
    digitalWrite(PIN_MOTOR2_PWM, LOW);
    } 

    Serial.println("");

  //To do: throttle change limiting, to avoid radical changes of direction for large DC motors


}