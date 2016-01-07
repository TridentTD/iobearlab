#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "MeshishNode.h"

#ifndef IP_FORWARD
#define IP_FORWARD    1
#endif


// Password ของทั้งระบบ node ทุกตัวตั้งค่าเดียวกันที่นี่
const char* pwd = "";

MeshishNode node;

struct AccessPoint {
    String ssid;
    long rssi;
    byte encrypt;
    unsigned int nodeType;
  };

void setup() {
  Serial.begin(115200);
  node.debug(&Serial);

  node.setup(pwd, true);
}

void loop() {
  // put your main code here, to run repeatedly:
  node.loop();


  
}

