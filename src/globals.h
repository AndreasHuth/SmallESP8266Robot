/*
 * globals.h
 *
 *  Created on: 25.09.2017
 *      Author: anonymous
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <Arduino.h>

/*#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
*/
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

#include "defines.h"


extern unsigned long currentMillis;

extern bool S1_6_flag;
extern word SameStateCount;     //counter of non changed states - failsave
extern char S1_buff[3];         //servo socket buffer
extern char S2_buff[3];

extern ESP8266WebServer server;          // Create a webserver object that listens for HTTP request on port 80
extern WebSocketsServer webSocket;      // create a websocket server on port 81

extern byte FailSaveTime;


#endif /* GLOBALS_H_ */
