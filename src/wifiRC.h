/*
 * wifiRC.h.h
 *
 *  Created on: 25.09.2017
 *      Author: anonymous
 */

#ifndef WIFIRC_H_
#define WIFIRC_H_

#include <Arduino.h>
#include "defines.h"
#include "globals.h"

extern bool connected; 

void ResetToFactory(void);
void webSocketEvent(uint8_t , WStype_t , uint8_t * , size_t );

void initRC (void);


#endif /* WIFIRC_H_ */