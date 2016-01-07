#include "ESP8266WiFi.h"
#include <aREST.h>

enum nodeType{
  NODE_NONE,
  NODE_PRIMARY,
  NODE_SECONDARY
};

//---------Init Zone --------------------------
unsigned int node_type = NODE_PRIMARY;
String node_name = "NodeMain";
//unsigned int node_type = NODE_SECONDARY;
//String node_name = "NodeSecond";
String pwd       = "";


// Variables to be exposed to the API
int temperature;
int humidity;

//---------Public Zone-------------------------

struct AccessPoint {
  String ssid;
  long rssi;
  byte encrypt;
  unsigned int nodeType;
};

struct NAT {
  String station_ip;
  String station_name;
  int    station_forward;
};

//---------Private Zone-------------------------
int          _wifi_led = D4;
String       _ssid;
unsigned int _wifi_mode = (node_type==NODE_PRIMARY)? WIFI_AP : WIFI_STA;

unsigned int _secondary_status = 0;  // 0=ยังไม่พบ network  1=ระหว่างการ connecting 2=connectedแล้วแต่ยังไม่ได้สร้าง AP  3= สร้างเรียบร้อย
         int _wifi_connecting_loop= 5000/100;

#define MESHMINI_MAX_AP_SCAN 100
AccessPoint _apList[MESHMINI_MAX_AP_SCAN];
NAT         _node_nat[10];
int         _node_nat_size=0;

aREST       node_rest = aREST();
WiFiServer  server(80);

//----------------------------------------------

//*/==============================================

bool _gen_ssid(){
  _ssid = "";
  switch(node_type) {
    case NODE_PRIMARY:
      _ssid = "M_1_"+node_name; return true;
      break;
    case NODE_SECONDARY:
      _ssid = (_wifi_mode == WIFI_STA)? "" : "M_2_"+node_name; return true;
      break;
  }
  return false;
}

unsigned int _getNodeType(const AccessPoint& ap) {
  String _ssidPrefix="M";
  String prefix = ap.ssid.substring(0, _ssidPrefix.length());

  if (prefix.equals(_ssidPrefix)) {
    if (ap.ssid.substring(_ssidPrefix.length(), _ssidPrefix.length() + 3).equals("_1_")) {
      return NODE_PRIMARY;
    } else if (ap.ssid.substring(_ssidPrefix.length(), _ssidPrefix.length() + 3).equals("_2_")) {
      return NODE_SECONDARY;
    }
  }
  return NODE_NONE;
}

String _ipToString(const IPAddress& ip) {
  return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

void _scan_AP(){
  WiFi.mode(WIFI_STA); _ssid = "";
  digitalWrite(_wifi_led,HIGH);
  
  int _maxDBmPrimary = -1;
  
  Serial.println();
  Serial.println("---------------------------------------------");
  Serial.println("scan start... ");

  int _numNetworks = WiFi.scanNetworks();

  Serial.println( String(_numNetworks)+ " networks found");
  Serial.println();

  for(int i = 0; i < _numNetworks; i++) {
    _apList[i] = AccessPoint();
    _apList[i].ssid     = WiFi.SSID(i);             // ชื่อของ Access Point 
    _apList[i].rssi     = WiFi.RSSI(i);             // ความแรงของสัญญาณของ AP (หน่วยคือ dBm)
    _apList[i].encrypt  = WiFi.encryptionType(i);   // 
    _apList[i].nodeType = _getNodeType(_apList[i]); // เป็น Primary หรือ Secondary Node?

    if (_apList[i].nodeType == NODE_PRIMARY) {
      if ( _maxDBmPrimary == -1) {
        _maxDBmPrimary = i;
      } else {
        if (_apList[i].rssi > _apList[_maxDBmPrimary].rssi){
          _maxDBmPrimary = i;                       // index ของ AP จากที่เจอทั้งหมด ที่ มีสัญญาณแรงสุดๆ
        }
      }
    }
    Serial.print( String(i+1)+". "+String(_apList[i].ssid)+" ("+String(_apList[i].rssi)+" dBm)");
    Serial.print("   Encryption: "+ String(_apList[i].encrypt ));
    Serial.println("   nodeType: "+ String(_apList[i].nodeType));
  }

  if (_maxDBmPrimary != -1){
    Serial.println();
    Serial.println("connecting to...  "+ String( _apList[_maxDBmPrimary].ssid.c_str() ));
  }

  if ( pwd.equals("")) WiFi.begin(_apList[_maxDBmPrimary].ssid.c_str());
  else WiFi.begin(_apList[_maxDBmPrimary].ssid.c_str(), pwd.c_str());

  _secondary_status = 1;
  _wifi_connecting_loop = 5000 / 100;
}

//**/==============================================
void NAT_list(){
  // แสดงผล รวมทั้งหมดอีกรอบ
  Serial.println();
  Serial.println("--------- NAT ( "+ node_name +" )--------------");
  for(int i =0; i< _node_nat_size; i++){
    if( _node_nat[i].station_name != "" && _node_nat[i].station_ip != "") {
      Serial.println(_node_nat[i].station_name + "    " + _node_nat[i].station_ip+((_node_nat[i].station_forward)? " [FW]" : "[-]"));
    }
  }
  Serial.println();
  Serial.println("########################################");

}

void regist_gateway_NAT(String sta_name,String sta_ip, int sta_forward=0){
  switch(node_type) {
  case NODE_PRIMARY:
    break;
  case NODE_SECONDARY:
    WiFi.mode(WIFI_STA);
    String host = "192.168.4.1"; //_ipToString(WiFi.gatewayIP());
    WiFiClient client;
    if (!client.connect( host.c_str(), 80)) {
      Serial.println("connection failed");
      return;
    }
  
    // ส่งข้อมูลไปให้
    client.print(String("GET /NAT?params=") + String(sta_name)
      + "&ip=" +  String(sta_ip)   //_ipToString(WiFi.localIP())
      + "&fw=" +  sta_forward
      + " HTTP/1.1\r\n" 
      + "Host: " + host + "\r\n" 
      + "Connection: close\r\n\r\n");

    WiFi.mode(WIFI_AP_STA);
    break;
  }
  return;
}

int NODE_Handler(String command){
  // รูปแบบ get ที่รับมา "NODE?params=<station_ip>&var=<station_var>"
  Serial.println();
  Serial.println("GET params: "+command);
  int first_andsymbol = command.indexOf('&');

  String station_ip    = command.substring(0,first_andsymbol);
  String station_var   = command.substring(first_andsymbol+5);
  Serial.println(station_ip); //+ " : " +station_var);

    String host = station_ip;
    WiFiClient client;
    if (!client.connect( host.c_str(), 80)) {
      Serial.println("connection failed");
      return 0;
    }
  
    // ส่งข้อมูลไปให้
    client.print(String("POST ") //+station_var
      + " HTTP/1.1\r\n" 
      + "Host: " + host + "\r\n" 
      + "Connection: close\r\n\r\n");

  return 1;
}

int NAT_Handler(String command){
  // รูปแบบ get ที่รับมา "NAT?params=<station_name>&ip=<station_ip>&fw=<station_fw>"
  Serial.println();
  Serial.println("GET params: "+command);
  int first_andsymbol = command.indexOf('&');
  int second_andsymbol = command.indexOf('&',first_andsymbol+1);

  String station_name    = command.substring(0,first_andsymbol);
  String station_ip      = command.substring(first_andsymbol+4,second_andsymbol);
  int    station_forward = command.substring(second_andsymbol+4).toInt()+0;



  Serial.println();
  Serial.println("Nat_Handler");
  Serial.println("forward :"+ station_forward);

  bool found=false; int index_found=-1; int index_blank=-1;
  for(int i =0; i < _node_nat_size; i++) {
      // หากตัวแบบไม่ forward เคยพบแล้ว ตัวแบบ forward ip เดิมให้ลบทิ้ง
//    if( found ){
//      if( _node_nat[i].station_ip == station_ip
//           && _node_nat[i].station_forward == 1) {
//        _node_nat[i].station_name    = "";
//        _node_nat[i].station_ip      = "";
//        _node_nat[i].station_forward = 0;                
//      }
//    }
    if(_node_nat[i].station_name == station_name){
      if(found==false){
        // กรณีเจอ ชื่อที่เคยมี  ให้ update
        _node_nat[i].station_ip      = station_ip;
        _node_nat[i].station_forward = station_forward;

        found = true; index_found = i;
        Serial.println("UPDATING/RECONNECTING NAT... "); 
        Serial.println(station_name+" --> IP: "+station_ip + "("+station_forward+")");

      } else {
        // กรณีเจอซ้ำหลายที่ ที่หลังๆ ให้ลบทิ้ง delete ที่ซ้ำ
        _node_nat[i].station_name    = ""; 
        _node_nat[i].station_ip      = "";
        _node_nat[i].station_forward = 0;
      }
      if(_node_nat[i].station_name == "" && index_blank==-1) {
        // กรณีเจอ พื้นที่ว่าง
        index_blank= i;
      }
    }
  }

  // ทำการเพิ่ม NAT 
  if(found == false) { // กรณีไม่เคยมีอยู่เลย
    if(index_blank == -1) { index_blank = _node_nat_size++;} 

    //Serial.println( "ADDING NAT : index blank is... "+ String(index_blank));
    //Serial.println( "NAT size... "+ String(_node_nat_size));
    //Serial.println(station_name+" --> IP: "+station_ip);

    _node_nat[index_blank] = NAT();
    _node_nat[index_blank].station_name    = station_name;
    _node_nat[index_blank].station_ip      = station_ip;
    _node_nat[index_blank].station_forward = station_forward;

    Serial.println("ADDING NAT... ");
    Serial.println(station_name+" --> "+station_ip + "("+station_forward+")");
  }

  NAT_list();


  // ถ้า NODE ยังไม่ใช่ Primary ให้ส่งขึ้นไปยัง NODE สูงกว่าต่อไปอีก
  if( node_type == NODE_SECONDARY) {
    Serial.println("forwarding...");
    regist_gateway_NAT(station_name,_ipToString(WiFi.localIP()) , 1);
  }
  return 1;
}

//==============================================
void node_setup(){
  pinMode(_wifi_led,OUTPUT); digitalWrite(_wifi_led, HIGH);

  Serial.begin(115200);Serial.println();Serial.println();
  WiFi.softAPdisconnect();
  WiFi.disconnect();

  switch(node_type){
    case NODE_PRIMARY:
      if( _gen_ssid() ) {
        WiFi.mode(WIFI_STA);delay(10000);WiFi.mode(WIFI_AP);
        WiFi.softAP( _ssid.c_str(), pwd.c_str() );
        digitalWrite(_wifi_led,LOW);

        Serial.println( "Primary Node AP : \"" + _ssid + "\" started");
        //Serial.println( "IP_FORWARD : " + String(IP_FORWARD));

        node_rest.set_id("1");
        node_rest.set_name(node_name);
        Serial.println("node rest id : 1   name :"+ node_name);

        NAT_list();

        node_rest.function("NAT",NAT_Handler);
        node_rest.function("NODE",NODE_Handler);
      }else { Serial.println(" _gen_ssid() : false "); }
      break;
    case NODE_SECONDARY:
      WiFi.mode( WiFiMode(_wifi_mode));
      _scan_AP();
      break;
  }

  // Start the server
  server.begin();
  Serial.println();
  Serial.println("Server started");

}

//==============================================
void node_loop(){
  switch(node_type){
    case NODE_PRIMARY:
      break;
    case NODE_SECONDARY:
      int status = WiFi.status();
      if( status != WL_CONNECTED ) {
        _secondary_status = 1;
      }
      if(_wifi_connecting_loop<0) {
        _secondary_status = 0;
        Serial.println("scan again");
      }

      
      switch(_secondary_status){
        case 0: //ยังไม่ได้พบ networks ที่ต้องการ
          _scan_AP();
          break;
        case 1: //ระหว่างการ connecting เข้า network ที่ต้องการ
          while( status != WL_CONNECTED && (_secondary_status==1) ) {
            status = WiFi.status();
            Serial.print(".");
            if( _wifi_connecting_loop-- <0) { _secondary_status = 0; }
            digitalWrite(_wifi_led,LOW); delay(50); digitalWrite(_wifi_led,HIGH); delay(50);
          }
          
          if( _wifi_connecting_loop-- <0) {
            Serial.println();
            Serial.println("can not connect");
            break;
          }else {
            Serial.println();
            Serial.println("connected!");
            Serial.println("AP      IP : "+ _ipToString(WiFi.softAPIP()));
            Serial.println("Local   IP : "+ _ipToString(WiFi.localIP()));  
            Serial.println("Gateway IP : "+ _ipToString(WiFi.gatewayIP()));
            Serial.println();
            
            _secondary_status = 2; _wifi_mode = WIFI_AP_STA;

            regist_gateway_NAT(node_name,_ipToString(WiFi.localIP()),0);

          }
          break;
        case 2: // connected แล้ว และเตรียมสร้าง softAP
          if( _gen_ssid() ) {
            WiFi.softAP( _ssid.c_str(), pwd.c_str() );
            digitalWrite(_wifi_led,LOW);
    
            Serial.println( "Secondary Node AP : \"" + _ssid + "\" started");

            IPAddress ip = WiFi.localIP();
            node_rest.set_id(String(ip[3]));
            node_rest.set_name(node_name);
            Serial.println("node rest id :"+ String(ip[3])+"  name :"+ node_name);

            NAT_list();        
            node_rest.function("NAT",NAT_Handler);
      
          }else { Serial.println(" _gen_ssid() : false "); }
          _secondary_status = 3;
          break;
        case 3: // เสร็จสิ้นทุกขั้นตอน
          break;
      }
      break;
  }


  // Handle REST calls
  WiFiClient client = server.available();
  node_rest.handle(client);
}
//==============================================
void setup() {
  // put your setup code here, to run once:
  node_setup();

  temperature = 24;
  humidity = 40;
  node_rest.variable("temperature",&temperature);
  node_rest.variable("humidity",&humidity);

  
}

void loop() {
  temperature = random(30,50);
  humidity    = random(60,90);

  
  // put your main code here, to run repeatedly:
  node_loop();
}

