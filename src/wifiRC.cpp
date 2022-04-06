/*
   fileSystem.cpp

    Created on: 25.09.2017
        Author: anonymous
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h> //for parsing/saving json file
#include <LittleFS.h>   // Include the LittleFS library

#include "defines.h"
#include "lib.h"

#include <FS.h>   // Include the FS library

#include "defines.h"
#include "globals.h"

ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80
WebSocketsServer webSocket = WebSocketsServer(81);    // create a websocket server on port 81

bool connected = false; 

bool S1_6_flag = false;
word SameStateCount = 0; //counter of non changed states - failsave
char S1_buff[3];//servo socket buffer
char S2_buff[3];

byte FailSaveTime = 5;

char  RCSSID[64] = "SSID";//WiFi.macAddress().ToString()+" (miharix.eu)";
char  RCPASS[64] = "miharix.eu";

bool WiFiAP = true;             //make WiFi AP 1 or connect to wifi 0
bool factory_boot = false;

bool ResetToFactoryFlag = false;

String getContentType(String filename); // convert the file extension to the MIME type
bool handleFileRead(String path);       // send the right file to the client (if it exists)

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);


void ResetToFactory() {
  //Serial.println("HARD_RESET_PIN: "+String(digitalRead(HARD_RESET_PIN)));
  if (!digitalRead(HARD_RESET_PIN) || ResetToFactoryFlag) {
    digitalWrite(2, LOW);
    delay(50);
    for (uint8_t t = 50; t > 0; t--) { //blink before hard reset
      digitalWrite(2, HIGH);
      delay(t * 10);
      digitalWrite(2, LOW);
      delay(t * 10 / 2);
    }
    if (!digitalRead(HARD_RESET_PIN) || ResetToFactoryFlag) {
#ifdef DEBUG1
      Serial.println("GO HARD RESET");
#endif
      for (uint8_t t = 5; t > 0; t--) { //blink before hard reset
        digitalWrite(2, HIGH);
        delay(100);
        digitalWrite(2, LOW);
        delay(100);
      }

      //----------
      File RestoreFile = SPIFFS.open("/config_restore.json", "r");
      File RewriteFile = SPIFFS.open("/config.json", "w");
      while (RestoreFile.available()) {
        digitalWrite(2, HIGH);
        RewriteFile.write(RestoreFile.read());
        digitalWrite(2, LOW);
      }
      RestoreFile.close();
      RewriteFile.close();
#ifdef DEBUG1
      Serial.println("GO HARD RESET done");
#endif
      for (uint8_t t = 20; t > 0; t--) { //blink before hard reset
        digitalWrite(2, HIGH);
        delay(50);
        digitalWrite(2, LOW);
        delay(50);
      }
      delay(2000);
      ESP.restart();
    }
  }
}

void print_variables() {
  PRINTLN(FailSaveTime);
  PRINTLN(RCSSID);
  PRINTLN(RCPASS);
  PRINTLN(WiFiAP);
}


String getContentType(String filename) { // convert the file extension to the MIME type

  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/html";
}

bool saveConfig() {
  print_variables();
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    return false;
  }
  StaticJsonBuffer<1124> jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();

  if (!factory_boot) {
    json["SSID"] = RCSSID;
  } else {
    json["SSID"] = "SSID";
  }
  json["Pass"] = RCPASS;
  json["WiFiAP"] = WiFiAP;
  json["Ftime"] = FailSaveTime;

  if (json.prettyPrintTo(configFile) == 0) {
#ifdef DEBUG1
    Serial.println(F("Failed to write to file"));
#endif
  }

  // Close the file (File's destructor doesn't close the file)
  configFile.close();
  return true;
}

bool loadConfig() {

  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    return false;
  }
  size_t size = configFile.size();

#ifdef DEBUG1
  Serial.print("Size: ");
  Serial.println(size);
#endif
  if (size > 1124) {
#ifdef DEBUG1
    Serial.println("Config to big");
#endif
    return false;
  }
  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);
#ifdef DEBUG
  Serial.println("buffer: ");
  Serial.println(buf.get());
#endif
  StaticJsonBuffer<1124> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());
  if (!json.success()) {
#ifdef DEBUG1
    Serial.println("parser error");
#endif
    return false;
  }
  configFile.close();//close the file

  FailSaveTime = json["Ftime"];

#ifdef DEBUG1
  // Serial.println(RCSSID);
#endif
  if (json["SSID"] != "SSID") {
    strlcpy(RCSSID, json["SSID"], sizeof(RCSSID));
    strlcpy(RCPASS, json["Pass"], sizeof(RCPASS));
    WiFiAP = json["WiFiAP"];
  } else {//ssid not set go AP use mac and display pass
    unsigned char masrc[6];
    WiFi.macAddress(masrc);
    String macRC = String(masrc[0], 16) + ":" + String(masrc[1], 16) + ":" + String(masrc[2], 16) + ":" + String(masrc[3], 16) + ":" + String(masrc[4], 16) + ":" + String(masrc[5], 16) + " (miharix.eu)";
#ifdef DEBUG1
    Serial.println(macRC);
#endif
    macRC.toCharArray(RCSSID, sizeof(RCSSID));
    strlcpy(RCPASS, "miharix.eu", sizeof(RCPASS));
    WiFiAP = true;
    factory_boot = true; //first time boot -> on root go to FAQ.html
  }

  print_variables();
  return true;
}

void rebooESP(String filename) {
  //server.send(200, "text/html", "<html><head><meta http-equiv='refresh' content='60;url=/setup.html' /></head><body><h1>Auto <a href='/setup.html'>reload</a> in 60 seconds...</h1></body></html>");

  File file = SPIFFS.open(filename, "r");                    // Open the file
  size_t sent = server.streamFile(file, "text/html");    // Send it to the client
  file.close();                                          // Close the file again

  delay(2000);
  ESP.restart();
}

void handleargs() { //handle http_get arguments
  for (int i = 0; i < server.args(); i++) { //for debug print all arguments
#ifdef DEBUG1
    Serial.println("Arg " + server.argName(i) + "=" + String(server.arg(i)));
#endif
  }
  //update temp variables

  if (server.hasArg("Ftime")) {
    FailSaveTime = server.arg("Ftime").toInt();
  }

  if (server.hasArg("RCSSID")) {
    char  RCSSID_temp[64] = "SSID_temp";
    server.arg("RCSSID").toCharArray(RCSSID_temp, sizeof(RCSSID_temp));
    // RCSSID = server.arg("RCSSID").c_str();
    if (RCSSID_temp != "SSID") {
      factory_boot = false;
      server.arg("RCSSID").toCharArray(RCSSID, sizeof(RCSSID));

      if (server.hasArg("RCPASS")) {
        server.arg("RCPASS").toCharArray(RCPASS, sizeof(RCPASS));
        //RCPASS = server.arg("RCPASS").c_str();
      }
      if (server.hasArg("WiFiAP")) {
        //WiFiAP = server.arg("WiFiAP");
        server.arg("WiFiAP");
        
        //RCPASS = server.arg("RCPASS").c_str();
      } else {
        WiFiAP = false;
      }
    }
  }

  if (server.hasArg("save")) { //detect button click
    switch (server.arg("save").toInt()) {
      case 1:
#ifdef DEBUG1
        Serial.println("RAM");//use only temp varible
#endif

        delay(500);
        print_variables();
        break;
      case 2:
        if (saveConfig())
#ifdef DEBUG1
          Serial.println("EEPROM saved");//save temp variables to json file
#endif
        break;
      case 3:
#ifdef DEBUG1
        Serial.println("REBOOT");
#endif
        //rebooESP("/reboot.html");
        break;
      case 4:
        //rebooESP("/restore.html");
        ResetToFactoryFlag = true;
        ResetToFactory();
#ifdef DEBUG1
        Serial.println("RESET ALL");//back to "factory defoults"
#endif
        break;
      case 5:
        if (saveConfig()) {
#ifdef DEBUG1
          Serial.println("EEPROM saved");//save temp variables to json file
          Serial.println("SET WiFi restarting");
#endif
          //rebooESP("/SSID_pass.html");
        } else {
          //rebooESP("/reboot.html");
        }

        break;
    }
  }
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  if (server.args() != 0) {
    handleargs();
  }
#ifdef DEBUG1
  Serial.println("handleFileRead: " + path);
#endif
  if (path.endsWith("/")) {
    if (factory_boot) { //factory boot show faq
      path += "faq";
    } else {
      path += "control";          // If a folder is requested, send the index file
    }
  }
  if (path.endsWith("/conf") || path.endsWith("/config") || path.endsWith("/edit") || path.endsWith("/nastavi") || path.endsWith("/uredi") || path.endsWith("/root") || path.endsWith("/admin") || path.endsWith("/administrator")) {
    path = "/setup";
  }
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  String pathWithHtml = path + ".html";


  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path) || SPIFFS.exists(pathWithHtml)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz)) {                        // If there's a compressed version available
      path += ".gz";
    } else {                         // Use the compressed version
      if (SPIFFS.exists(pathWithHtml)) {
        path += ".html";
      } else {
      }
    }
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
#ifdef DEBUG1
    Serial.println(String("\tSent file: ") + path);
#endif
    return true;
  }
#ifdef DEBUG1
  Serial.println(String("\tFile Not Found: ") + path);
#endif
  return false;                                          // If the file doesn't exist, return false
}


void initRC (void)
{
  pinMode(HARD_RESET_PIN, INPUT);
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  SPIFFS.begin();                           // Start the SPI Flash Files System
  loadConfig();
  delay(10);


  if (!WiFiAP) { //Connect to wifi
    WiFi.begin(RCSSID, RCPASS);
#ifdef DEBUG1
    Serial.print("Connecting to ");
    Serial.print(RCSSID); Serial.println(" ...");
#endif
    int NC_count = 0;
    while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
      delay(1000);
#ifdef DEBUG1
      Serial.print('.');
#endif
      if (NC_count > 300) {
        ResetToFactoryFlag = true;
        ResetToFactory();
      }
      NC_count++;
    }
#ifdef DEBUG1
    Serial.println('\n');
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());              // Tell us what network we're connected to
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer
#endif
  } else { //go AP wifi mode
    WiFi.softAP(RCSSID, RCPASS);             // Start the access point
#ifdef DEBUG1
    Serial.print(RCSSID);
    Serial.print(" Access Point \"");
    Serial.println("\" started");
    Serial.print("IP address:\t");
    Serial.println(WiFi.softAPIP());         // Send the IP address of the ESP8266 to the computer
#endif

  }

  if (MDNS.begin("rc")) {              // Start the mDNS responder for rc.local
#ifdef DEBUG1
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
#endif
  }

  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.begin();                           // Actually start the server
#ifdef DEBUG1
  Serial.println("HTTP server started");
#endif
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
#ifdef DEBUG1
  Serial.println("websocket server started");
#endif

  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);

  connected = false;
}


//##############################################################################################

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) {
    case WStype_DISCONNECTED:

#ifdef DEBUG2
      Serial.printf("[%u] Disconnected!\n", num);
#endif
      connected = false; 

      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);

#ifdef DEBUG2
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
#endif
        connected = true; 

        // send message to client
        webSocket.sendTXT(num, "Connected");
        buzzerOn200ms();
      }
      break;
    case WStype_TEXT:

#ifdef DEBUG2
      // Serial.printf("[%u] get Text: %s\n", num, payload);
#endif

      if ((payload[0] == 'S') && (payload[13] == '\n')) {

        memcpy(S1_buff, &payload[1], 2 );
        // S1_buff[3] = '\0';

        memcpy(S2_buff, &payload[3], 2 );
        //     S2_buff[3] = '\0';
        
        S1_6_flag = true;

        if (S1_buff[0] == '-') { //prevent buffer owerflow
          S1_buff[0] = '0';
          S1_buff[1] = '0';
        }
        if (S2_buff[0] == '-') {
          S2_buff[0] = '0';
          S2_buff[1] = '0';
        }
       }
      break;
    case WStype_ERROR: break;
    case WStype_BIN: break;
    case WStype_FRAGMENT_TEXT_START: break;
    case WStype_FRAGMENT_BIN_START: break;
    case WStype_FRAGMENT: break;
    case WStype_FRAGMENT_FIN: break;
    case WStype_PING: break;
    case WStype_PONG: break;
  }
}