// The MIT License (MIT)
//
// Copyright (c) 2015 Brannon Dorsey <brannon@brannondorsey.com>
//               with support from the SAIC OpenLab
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef __MESHISH_H__
#define __MESHISH_H__

#define MESHISH_HTTP_PORT 80
#define MESHISH_PRIMARY_IP "192.168.4.1"
#define MESHISH_MAX_AP_SCAN 100

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"

class MeshishNode {

public:

  struct AccessPoint {
    String ssid;
    long rssi;
    byte encrypt;
    unsigned int nodeType;       // Primary Node (1) or Secondary Node (0)
//   unsigned int nodeAttribute;  // Attribute 1= Master_Node,  Attribute 2 = Repeater_Node ,  Attribute 3 = End_Node
    
  };

  enum nodeType{
    NODE_NONE,
    NODE_PRIMARY,
    NODE_SECONDARY
  };

  enum nodeAttribute{
    NODE_ATTB_MASTER,
    NODE_ATTB_REPEATER,
    NODE_ATTB_ENDING
  };


  MeshishNode();
  ~MeshishNode();

  void setup(String password, bool primary=false, int nodeAttribute=2) ;
  void loop();
  void makePrimary(bool primary=true);
  void debug(HardwareSerial* serial);
  bool isPrimary();
  // bool isConnectedToPrimary();
  // bool enableDefaultRoutes(bool b=true);
  unsigned int getStatus();

//  unsigned int getNodeAttribute();
//  void setNodeAttribute(int nodeAtrib=2); // Attribute 1= Master_Node,  Attribute 2 = Repeater_Node ,  Attribute 3 = End_Node


protected:

  void _setSSID();
  void _scanAndConnect();
  bool _generateSSID();
  unsigned int _getNodeType(const AccessPoint& ap);
  String _ipToString(const IPAddress& ip);

  int  _getWIFIMODE();
  void _setWIFIMODE(int wifimode);
  

  byte _encrypt;
  bool _isPrimary;
  bool _debug;
  bool _showConnected=false;
  bool _showCreatedAP=false;
  bool _showDisconnected=false;

  int  _nodeAttribute;  //  Attribute ไหน ( Master node : 1, Repeater node : 2, Ending node : 3 )
  bool _connectedToAP; 

  // node แบบ 1 (Master Node)  กำหนดเป็น Primary ต้นทาง WIFI_AP อย่างเดียว0
  // node แบบ 2 (Repeater Node)หากยังไม่เชื่อมAP จะมี WiFi.mode เป็น WIFI_STA, หากเชื่อมAPแล้ว  WiFi.mode จะกลายเป็น WIFI_AP_STA เพื่อรอให้ node แบบ 3 ต่อเชื่อมต่อไป
  // node แบบ 3 (Ending Node)  กำหนดเป็นปลายทางคือ WIFI_STA อย่างเดียว ไม่มีหน้าที่เป็น APต่อไป
  
  int  _currentWIFIMODE;
  
  bool _createdAP; // true when AP is being created
  unsigned int _numNetworks;
  uint32_t _chipId;
  String _ssid;
  String _ssidPrefix;
  String _password;
  
  ESP8266WebServer _server;
  AccessPoint _apList[MESHISH_MAX_AP_SCAN];
  HardwareSerial* _serial;

  int _connecting_loop = 5 * 1000 / 50;
  int WIFI_LED_PIN = D4;
  //int WIFI_LED_PIN =BUILTIN_LED;

};

#endif /* __MESHISH_H__ */

