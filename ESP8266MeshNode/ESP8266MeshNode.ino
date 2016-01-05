#include "Arduino.h"
#include "ESP8266WiFi.h"
//#include "ESP8266WebServer.h"
#include <aREST.h>
#include <ESP8266HTTPClient.h>

//////////////////////////////////////////////////
#define MESHISH_HTTP_PORT 80
#define MESHISH_PRIMARY_IP "192.168.4.1"
#define MESHISH_MAX_AP_SCAN 100

// NODE TYPE
#define NODE_NONE      0
#define NODE_PRIMARY   1
#define NODE_SECONDARY 2


bool   _debug=true;

//int  NODE_TYPE = NODE_PRIMARY;
int  NODE_TYPE = NODE_SECONDARY;

String password="";

//String rest_id = "01";
//String rest_name= "PumpCenter";

String rest_id = "03";
String rest_name= "kuti03";

int temperature;
int humidity;
String externalAP, internalAP;
//////////////////////////////////////////////////

aREST rest = aREST();

String _ssid; 
String _password;
//ESP8266WebServer _webserver(80);
WiFiServer _webserver(80);
//int    _port=-1;


uint32_t _chipID;
bool     _primary=false;
int      _nodeType =NODE_TYPE;
int      _wifi_mode = ( _nodeType == NODE_SECONDARY)? WIFI_STA : WIFI_AP;
bool     _wifi_connected=false;

bool     _showConnectingInfo = false;
bool     _showConnectedInfo = false;

String _ssidPrefix="M";


struct NAT {
  String nat_name;
  IPAddress nat_ip;
};

NAT _NetworkAddressTable[10];

struct AccessPoint {
  String ssid;
  long rssi;
  byte encrypt;
  unsigned int nodeType;  // nodeType 1 = Primary, nodeType2= Secondary
};

AccessPoint _apList[MESHISH_MAX_AP_SCAN];

int _connecting_loop = 5 * 1000 / 50;
int  WIFI_LED_PIN     = D4;

//************************************************
//*** These functions are called only from node_setup() and node_loop() only
//************************************************
String _ipToString(const IPAddress& ip) { return String(ip[0]) + "." +  String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]); }

unsigned int _getNodeType(const AccessPoint& ap){
  String prefix = ap.ssid.substring(0, _ssidPrefix.length());

  if (prefix.equals(_ssidPrefix)){
    if (ap.ssid.substring(_ssidPrefix.length(), _ssidPrefix.length() + 3).equals("_1_")){
      return NODE_PRIMARY;
    } else if (ap.ssid.substring(_ssidPrefix.length(), _ssidPrefix.length() + 3).equals("_2_")) {
      return NODE_SECONDARY;
    }
  }

  return NODE_NONE;
}

void _webserver_begin(){  
   //_webserver.on("/",  handleRoot );
//  _webserver.on("/", []()   { _webserver.send(200, "text/html",  "welcome! "+ _chipID );  });
//  _webserver.onNotFound([](){ _webserver.send(200, "text/plain", "404 file not found");  });
  _webserver.begin();

  if(_debug){Serial.println("Webserver begin!");}
}


void _scanAndConnect() {
  //_webserver.stop();  //ต้อง library รุ่น github จึงจะมี่
  
  int _numNetworks   = 0;
  int _maxDBmPrimary = -1; // the index of the node with the highest rssi
  _wifi_connected     = false;
  _showConnectingInfo = false;
  _showConnectedInfo  = false;

  digitalWrite(WIFI_LED_PIN,HIGH); //กำหนดไฟบนบอร์ด ESP  ให้ดับ 
  if (_debug){
    Serial.println();
    Serial.println("---------------------------------------------------------------");
    Serial.println("scan start...");
  }
  
  _numNetworks = (WiFi.scanNetworks() < MESHISH_MAX_AP_SCAN)? WiFi.scanNetworks(): MESHISH_MAX_AP_SCAN;
  if (_debug) { Serial.print(_numNetworks); Serial.println(" networks found in scan"); Serial.println(); }

  // Scan all WiFi AP
  for (int i = 0; i < _numNetworks; i++) {
    _apList[i]          = AccessPoint();
    _apList[i].ssid     = WiFi.SSID(i);
    _apList[i].rssi     = WiFi.RSSI(i);
    _apList[i].encrypt  = WiFi.encryptionType(i);
    _apList[i].nodeType = _getNodeType(_apList[i]);  // 0 คือ AP นอกระบบ Mesh, 1 คือ AP ที่เป็น Primary, 2 คือ AP ที่เป็น Secondary

//    if (_apList[i].nodeType == NODE_PRIMARY) {
      if ( _maxDBmPrimary == -1) { _maxDBmPrimary = i; }
      else{ if (_apList[i].rssi > _apList[_maxDBmPrimary].rssi) { _maxDBmPrimary = i;} }
//    } else if (_apList[i].nodeType == NODE_SECONDARY) {
//      if ( _maxDBmPrimary == -1) { _maxDBmPrimary = i; }
//      else{ if (_apList[i].rssi > _apList[_maxDBmPrimary].rssi) { _maxDBmPrimary = i;} }
//    }

    if (_debug) {
      Serial.print(i+1); Serial.print(") "); Serial.print(_apList[i].ssid); Serial.print(" (");
      Serial.print(_apList[i].rssi);  Serial.print("dBm)");
      Serial.print("\tEncryption: "); Serial.print(_apList[i].encrypt);
      Serial.print("\tnodeType: ");   Serial.println( _apList[i].nodeType);
    }
  }

  // begin Best WiFi
  if(_maxDBmPrimary != -1 ) {
    if (_debug) {
      Serial.println("");
      Serial.print("_scanAndConnect(): connecting to ");
      Serial.println(_apList[_maxDBmPrimary].ssid.c_str());
    }

    if (_password.equals("")) WiFi.begin(_apList[_maxDBmPrimary].ssid.c_str());
    else WiFi.begin(_apList[_maxDBmPrimary].ssid.c_str(), _password.c_str());

    externalAP = _apList[_maxDBmPrimary].ssid;

    _webserver_begin();
  }
  _ssid = "";  //reset ชื่อ AP เตรียมไว้ก่อน
}

bool _generateSSID(){
  if(_primary) {
    //_ssid = String(_ssidPrefix +"_1_" + String(MESHISH_PRIMARY_IP) + "_" + String(_chipID)); 
    _ssid = String(_ssidPrefix +"_1_" + String(MESHISH_PRIMARY_IP) + "_" + rest_name );
    return true;
  } else {
    if( _wifi_connected ) {
      //_ssid = String( _ssidPrefix +"_2_" + _ipToString(WiFi.localIP()) + "_" + String(_chipID));
      _ssid = String(_ssidPrefix +"_2_" + _ipToString(WiFi.localIP()) + "_" + rest_name ); 
      return true;
    }
  }

  return false;
}

// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(D4,state);
  return 1;
}

// Custom function accessible by the API ... NETWORK ADDRESS TABLE ..(NAT)
int NATControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(D4,state);
  return 1;
}


//************************************************
//================================================
//void node_setup(String password="", int nodeType=NODE_SECONDARY,int port=-1 ) {
void node_setup(String password="", int nodeType=NODE_SECONDARY ) {
  _password = password;
  _nodeType = nodeType;
  _chipID   = ESP.getChipId();

  //_port = (port>0)? port : ((_nodeType == NODE_PRIMARY)? 3000 : _port);

  Serial.begin(115200);
  pinMode(WIFI_LED_PIN,OUTPUT);
  digitalWrite(WIFI_LED_PIN,HIGH); //ให้ไฟดับเสียก่อน


  WiFi.disconnect(); delay(100);
  switch( _nodeType){
  case NODE_PRIMARY :
    _primary = true;  _wifi_mode=WIFI_AP; 
    
    WiFi.mode(WiFiMode(_wifi_mode));
    if(_generateSSID()){
      WiFi.softAP( _ssid.c_str(), _password.c_str() );
      internalAP =_ssid;
      if (_debug) {
        Serial.println();
        Serial.print("AP IP   : "); Serial.println(WiFi.softAPIP());
        Serial.print("local IP: "); Serial.println(WiFi.localIP());
      }
    }

    _webserver_begin();
    break;
  case NODE_SECONDARY :
    _primary = false; _wifi_mode=WIFI_STA; 
    WiFi.mode(WiFiMode(_wifi_mode));
    _scanAndConnect();


    
    break;
  }

  
}

void node_loop(){
  if(_nodeType == NODE_SECONDARY){
    while( WiFi.status() != WL_CONNECTED ) {
      _showConnectedInfo = false;
      if(_debug) {
        if(!_showConnectingInfo) {
          if (WiFi.status()     == WL_CONNECTION_LOST)  { _showConnectingInfo=true; Serial.println("node_loop: connection lost"); }
          else if(WiFi.status() == WL_DISCONNECTED)     { _showConnectingInfo=true; Serial.println("node_loop: wait for connecting");    }
        }else{
          if (WiFi.status()     == WL_CONNECTION_LOST)  { _showConnectingInfo=true; Serial.print("*"); }
          else if(WiFi.status() == WL_DISCONNECTED)     { _showConnectingInfo=true; Serial.print(".");}
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
    if( !_showConnectedInfo) {
      if (WiFi.status() == WL_CONNECTED) {
        WiFi.mode(WIFI_STA);
        _wifi_connected = true; _showConnectingInfo = false;
        if(_debug) {
          Serial.println(); 
          Serial.print("node_loop: connection to \""); Serial.print(WiFi.SSID()); Serial.println("\" established.");
          Serial.print("node_loop: AP IP      :");      Serial.println(WiFi.softAPIP());
          Serial.print("node_loop: gateway IP :");      Serial.println(WiFi.gatewayIP());
          Serial.print("node_loop: local IP   :");      Serial.println(WiFi.localIP()); 
          _showConnectedInfo = true;
        }

        // หลัง connect เข้าสู่ externalAP ได้แล้ว ต่อไปสร้าง internalAP ในวงตัวเอง
//        if( _nodeType == NODE_SECONDARY) { 
//          WiFi.mode(WIFI_AP_STA);
//          if(_generateSSID()){
//            WiFi.softAP( _ssid.c_str(), _password.c_str() );
//            internalAP =_ssid;
//            
//            if (_debug) {
//              Serial.println();
//              Serial.println("created new AP \"" + _ssid + "\"");
//              Serial.print("node_loop: AP IP      :");      Serial.println(WiFi.softAPIP());
//              Serial.print("node_loop: gateway IP :"); Serial.println(WiFi.gatewayIP());
//              Serial.print("node_loop: local IP   :");      Serial.println(WiFi.localIP()); 
//              //if(_port>0){ Serial.print("PORT : "); Serial.println(_port); }
//            }
//          }
//        }
      }
    }
  }
  
  //_webserver.handleClient();

    // Handle REST calls
  WiFiClient client = _webserver.available();
  if (!client) {
    return;
  }
  uint16_t maxWait=1000;
  while(client.connected() && !client.available() && maxWait--){
    delay(1);
  }
  rest.handle(client);
}
//================================================
//////////////////////////////////////////////////
//////////////////////////////////////////////////
void setup() {
  // Init variables and expose them to REST API
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);
  
  if(_nodeType == NODE_SECONDARY){
    rest.variable("externalAP",  &externalAP);
    rest.variable("internalAP",  &internalAP);
  }else {
    rest.variable("internalAP",  &internalAP);
  }

    // Function to be exposed
  rest.function("led",ledControl);
  rest.function("NAT",NATControl);
  
  // Give name and ID to device
  rest.set_id(rest_id.c_str());
  rest.set_name(rest_name.c_str());

  //rest.set_status_led(D4);
  
  // put your setup code here, to run once:
  //node_setup(NODE_SECONDARY, node_name, password);
  //node_setup( password, NODE_SECONDARY);
  node_setup( password, NODE_TYPE);

}

void loop() {
  // put your main code here, to run repeatedly:
  temperature = random(10,30);
  humidity = random(40,90);

  node_loop();
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
