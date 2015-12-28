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

#include "MeshishNode.h"

MeshishNode::MeshishNode():
  _isPrimaryNode(false),
  _isSecondaryNode(false),
  _numNetworks(0),
  _ssidPrefix("M"),
  _ssid(""),
  _chipID(0),
  _serial(NULL),
  _debug(false),
  _connectedToAP(false),
  _createdAP(false),
  _password(""),
  //_server(ESP8266WebServer(80)),
  _port(-1)
{

}

MeshishNode::~MeshishNode()
{

}


//int handleRoot( c) {
//  return n->getChipID();
//}

// primary --> AP , secondary --> STA AP
// Type 1= Primary_Node,  Type 2 = Secondary_Node

void MeshishNode::setup(String password, int nodeType, int port ) 
{
  _debug = true;
  
  _password = password;
  _nodeType = nodeType;
  _chipID = ESP.getChipId();

  if( _nodeType == NODE_PRIMARY) { _port = 3000; }
  if( port > 0) { _port = port; }

  pinMode(WIFI_LED_PIN,OUTPUT);
  digitalWrite(WIFI_LED_PIN,HIGH); //ให้ไฟดับเสียก่อน
  
  //if(_isPrimary) { _nodeType = 1; }

  switch( _nodeType ) {
  case NODE_PRIMARY :
    _isPrimaryNode = true;
    _setWIFIMODE(WIFI_AP);
    break;
  case NODE_SECONDARY :
    _isSecondaryNode = true;
    _setWIFIMODE(WIFI_STA);  //เริ่มต้น เป็น WIFI_STA หลังต่อกับ AP แล้ว จะเปลี่ยนค่าเป็น WIFI_AP_STA
    break;
  }
  
  WiFi.disconnect();
  delay(100);

  bool ssidGenerated = _generateSSID();

  if (!ssidGenerated && _debug){  _serial->println("MeshishNode::setup: _generateSSID() returned false."); }

  if( _nodeType == NODE_PRIMARY ) {
    if (ssidGenerated) {  // กรณี node เป็นแบบ Primary_node
      if (_debug) {  _serial->println("MeshishNode::setup: Primary-Node creating SSID \"" + _ssid + "\""); }
      
      WiFi.softAP(_ssid.c_str());

      if (_debug) {
        _serial->println();
        _serial->print("AP IP   : "); _serial->println(WiFi.softAPIP());
        _serial->print("local IP: "); _serial->println(WiFi.localIP());
      }
      _createdAP = true;
    }
  } else {  // กรณี node เป็นแบบ SECONDARY NODE
    _scanAndConnect();
  }

  if(_debug){ 
    _serial->println(); _serial->print("PORT : "); _serial->println(_port); 
    _serial->println();
    _serial->print("Chip ID : "); Serial.println(getChipID());
    _serial->println();
    _serial->print("Primary Node?   "); (isPrimaryNode())?   Serial.println("true") : Serial.println("false");
    _serial->print("Secondary Node? "); (isSecondaryNode())? Serial.println("true") : Serial.println("false");
  }

  //_server.on("/",  handleRoot );
  _server.on("/", [this]()   { _server.send(200, "text/html",  "welcome!" );  });
  _server.onNotFound([this](){ _server.send(200, "text/plain", "404 file not found");  });
  _server.begin();

}

void MeshishNode::loop() {
  
  if(_nodeType==NODE_SECONDARY ) { // 
    while( getStatus() != WL_CONNECTED ) {
      if(_debug) {
        if(!_showDisconnected) {
          if (getStatus()     == WL_CONNECTION_LOST)  { _debug_showConnected=false; _showDisconnected = true; _serial->println(); _serial->println("MeshishNode::loop: connection lost"); }
          else if(getStatus() == WL_DISCONNECTED)     { _debug_showConnected=false; _showDisconnected = true; _serial->println(); _serial->println("MeshishNode::loop: wait for connecting...");    }
        }else{
          if (getStatus()     == WL_CONNECTION_LOST)  { _debug_showConnected=false; _showDisconnected = true; _serial->print("*"); }
          else if(getStatus() == WL_DISCONNECTED)     { _debug_showConnected=false; _showDisconnected = true; _serial->print(".");}
        }
      }
      digitalWrite(WIFI_LED_PIN,HIGH); delay(50); digitalWrite(WIFI_LED_PIN,LOW); delay(50); 
      _connecting_loop--;
      if( _connecting_loop < 0) {
        digitalWrite(WIFI_LED_PIN,HIGH); 
        _connecting_loop = 5 * 1000 / 50;
        _scanAndConnect();
      }
    }
  
    if(!_debug_showConnected) {
      if(getStatus()  == WL_CONNECTED)  { 
        _connectedToAP = true;  _debug_showConnected = true; _showDisconnected = false; 
        if(_debug) {
          _serial->println(); 
          _serial->print("MeshishNode::loop: connection to \""); _serial->print(WiFi.SSID()); _serial->println("\" established.");
          _serial->print("MeshishNode::loop: recieved IP Address "); _serial->println(WiFi.localIP());
        }

        if( _nodeType == NODE_SECONDARY) { _setWIFIMODE(WIFI_AP_STA); }
      }
    }

//    while( getStatus() == WL_CONNECTED ) {
//      
//    }
//
//    _connectedToAP = false;  _debug_showConnected = true; _showDisconnected = false; 
  }


//    
//    // connection established
//    if (status == WL_CONNECTED) {
//      _connectedToAP = false;
//      
//      if (_debug && (_debug_showConnected == false) ) {
//        _serial->println();
//        _serial->print("MeshishNode::loop: connection to \""); _serial->print(WiFi.SSID()); _serial->println("\" established.");
//        _serial->print("MeshishNode::loop: recieved IP Address "); _serial->println(WiFi.localIP());
//
//        _debug_showConnected = true; _showDisconnected=false;
//      }
//
//      
//      if(_nodeType==2 && _connectedToAP && !_createdAP) {
//        if(_createdAP==false) {
//          // now create a secondary AP
//          bool ssidGenerated = _generateSSID();
//    
//          if (ssidGenerated) {
//            // WiFi.softAPConfig(IPAddress(192, 168, 4, 10),
//            //                   IPAddress(192, 168, 4, 10),
//            //                   IPAddress(255, 255, 255, 0)); 
//            WiFi.softAP(_ssid.c_str());
//            _createdAP = true;
//          }
//        }
//      }
//    } else {  // เคย connected เข้า AP แล้วหลุดไป 
//      
//      _connectedToAP = false; _createdAP = false; _debug_showConnected = false;_debug_showCreatedAP = false;
//      
//      //if(_nodeType==2 ) { _setWIFIMODE(WIFI_STA);  }
//      
//
//
//
////      if (status == WL_CONNECT_FAILED) {  
////        if (_debug) { _serial->print("MeshishNode::loop: connection to "); _serial->print(WiFi.SSID()); _serial->println(" failed."); }
////        
////      } // a temporary status before WiFi.begin()
////      else if (status == WL_IDLE_STATUS){
////        if (_debug) { _serial->println("MeshishNode::loop: connection idle..."); }
////        _debug_showConnected = false;
////      }
//    }
//  } else {
//    // not _connectedToAP
////    if( _nodeType != 1 ) {
////      _scanAndConnect();
////    }
//  }
//  
//  ////////////////////////////////////////////////////////////
//  if (_createdAP){
//
//    if (_debug && !_debug_showCreatedAP ) {
//      _serial->println("MeshishNode::loop: Access point created.");
//      WiFi.printDiag(*_serial);
//      _debug_showCreatedAP = true;
//    }
//  } else {
//    _debug_showCreatedAP = false;
//  }
//
  _server.handleClient();
}

void MeshishNode::debug(HardwareSerial* serial)
{
  _debug = true;
  _serial = serial;
}



//bool MeshishNode::isPrimary()
//{
//  return _isPrimary;
//}

bool MeshishNode::isPrimaryNode() {
  return _isPrimaryNode;
}

bool MeshishNode::isSecondaryNode() {
  return _isSecondaryNode;
}



// bool MeshishNode::isConnectedToPrimary()
// {

// }

// bool MeshishNode::enableDefaultRoutes(bool enable)
// {

// }

unsigned int MeshishNode::getStatus()
{
  return WiFi.status();
}

uint32_t MeshishNode::getChipID() {
  return _chipID;
}

void MeshishNode::_scanAndConnect()
{
  digitalWrite(WIFI_LED_PIN,HIGH); //กำหนดไฟบนบอร์ด ESP  ให้ดับ 
  if( _nodeType==NODE_SECONDARY) { _createdAP=false; _debug_showCreatedAP = false; }
  
  if (_debug){
    _serial->println();
    _serial->println("---------------------------------------------------------------");
    _serial->println("scan start...");
  }

  //_numNetworks = min(WiFi.scanNetworks(), MESHISH_MAX_AP_SCAN);
 // _numNetworks = (WiFi.scanNetworks(false,true) < MESHISH_MAX_AP_SCAN)? WiFi.scanNetworks(false,true): MESHISH_MAX_AP_SCAN;  // สำหรับ ESP8266 library จาก GitHub ล่าสุด สามารถสแกน network ที่ซ่อนได้
  _numNetworks = (WiFi.scanNetworks() < MESHISH_MAX_AP_SCAN)? WiFi.scanNetworks(): MESHISH_MAX_AP_SCAN;

  if (_debug) { _serial->print(_numNetworks); _serial->println(" networks found in scan"); _serial->println(""); }

  // the index of the node with the highest rssi
  int maxDBmPrimary = -1;

  for (int i = 0; i < _numNetworks; i++)
  {
    _apList[i] = AccessPoint();
    _apList[i].ssid     = WiFi.SSID(i);
    _apList[i].rssi     = WiFi.RSSI(i);
    _apList[i].encrypt  = WiFi.encryptionType(i);
    _apList[i].nodeType = _getNodeType(_apList[i]);  // 0 คือ AP นอกระบบ Mesh, 1 คือ AP ที่เป็น Master, 2 คือ AP ที่เป็น Repeater

    if (_apList[i].nodeType == NODE_PRIMARY)
    {
      if (maxDBmPrimary == -1)
      {
        maxDBmPrimary = i;
      }
      else
      {
        if (_apList[i].rssi > _apList[maxDBmPrimary].rssi)
        {
          maxDBmPrimary = i;
        }
      }
    }

    if (_debug)
    {
      _serial->print(i+1); _serial->print(") ");
      _serial->print(_apList[i].ssid);
      _serial->print("\t");
      _serial->print(_apList[i].rssi);
      _serial->print("dBm\t");
      _serial->print("Encryption: ");
      _serial->print(_apList[i].encrypt);
      _serial->print("\tnodeType: ");
      _serial->println(_apList[i].nodeType);
    }
  }

  if (maxDBmPrimary != -1)
  {
    if (_debug) {
      _serial->println("");
      _serial->print("MeshishNode::_scanAndConnect: connecting to ");
      _serial->println(_apList[maxDBmPrimary].ssid.c_str());
    } 

    if (_password.equals("")) WiFi.begin(_apList[maxDBmPrimary].ssid.c_str());
    else WiFi.begin(_apList[maxDBmPrimary].ssid.c_str(), _password.c_str());

    _connectedToAP= false; // WiFi เริ่มแล้ว แต่ยังไม่ได้ connect ต่อเข้าไป การ connect จะไปอยู่ใน loop() เพื่อวนเช็คว่า connect ติดหรือยังอีกที
    _debug_showConnected = false;
    
    //if(_nodeType ==NODE_SECONDARY && _connectedToAP ) { _setWIFIMODE(WIFI_AP_STA); }

  }
}

bool MeshishNode::_generateSSID()
{
  if (_isPrimaryNode)
  { 
    _ssid = String(_ssidPrefix + "_1_" + String(MESHISH_PRIMARY_IP) + "_" + String(_chipID));
    if (_debug)
    {
      _serial->print("MeshishNode::_generateSSID: _isPrimary, ssid:");
      _serial->println(_ssid);
    }
    return true;
  }
  else
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      IPAddress ip = WiFi.localIP();
      _ssid = String(_ssidPrefix + "_2_" + _ipToString(ip) + "_" + String(_chipID));
      if (_debug)
      {
      _serial->print("MeshishNode::_generateSSID: !_isPrimary, ssid:");
      _serial->println(_ssid);
      }
      return true;
    }
    else
    {
      if (_debug)
      {
      _serial->println("MeshishNode::_generateSSID: !_isPrimary but status != WL_CONNECTED");
      }
    }

  }

  return false;
}


unsigned int MeshishNode::_getNodeType(const AccessPoint& ap)
{
  String prefix = ap.ssid.substring(0, _ssidPrefix.length());

  if (prefix.equals(_ssidPrefix))
  {
    if (ap.ssid.substring(_ssidPrefix.length(), _ssidPrefix.length() + 3).equals("_1_"))
    {
      return NODE_PRIMARY;
    }
    else if (ap.ssid.substring(_ssidPrefix.length(), _ssidPrefix.length() + 3).equals("_2_"))
    {
      return NODE_SECONDARY;
    }
  }

  return NODE_NONE;
}

String MeshishNode::_ipToString(const IPAddress& ip)
{
  return String(ip[0]) + "." + 
         String(ip[1]) + "." + 
         String(ip[2]) + "." + 
         String(ip[3]);
}

int  MeshishNode::_getWIFIMODE(){
  return _currentWIFIMODE;
}

void MeshishNode::_setWIFIMODE(int wifimode){
  _currentWIFIMODE = wifimode;
  WiFi.mode(WiFiMode(wifimode));
}



