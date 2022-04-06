/*
 * Motors.cpp
 *
 *  Created on: 25.09.2017
 *      Author: anonymous
 */

#include "defines.h"
#include "globals.h"


// ########################################################################################

void buzzerOFF(void)
{
    digitalWrite(PIN_BUZZER, LOW);
}

// ########################################################################################

void initBuzzer(void)
{
    pinMode(PIN_BUZZER, OUTPUT);
    buzzerOFF();
}

// ########################################################################################

void buzzerOn200ms(void)
{
    // digitalWrite(PIN_BUZZER, HIGH);
    analogWrite(PIN_BUZZER, 150);
    delay(200);
    analogWrite(PIN_BUZZER, 0);
}

// ########################################################################################


int getLipoVoltage(void)
{
    int help = analogRead(A0);
    Serial.print("ADC: ");
    Serial.println(help);

    /*********************************************************
     * ULipo = ADC  *  (100k + 220k) / 100k  *  (10k + 10k) / 10k  *   1V /1023 
     * ULipo = ADC  *  3,2                   *  2                  /   1023   V
     * ULipo = ADC  *  0,006256109                                            V
     *********************************************************/
    #define U_FACTOR     0.006256109

    float voltage = (float) help;
    voltage *= U_FACTOR;
    Serial.print("U: ");
    Serial.println(voltage);
    if (voltage < 3.0)
        buzzerOn200ms();

    return voltage;
}