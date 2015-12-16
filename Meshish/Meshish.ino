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

#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "MeshishNode.h"

// Password ของทั้งระบบ node ทุกตัวตั้งค่าเดียวกันที่นี่
const char* pwd = "";

MeshishNode node;

void setup() {

  Serial.begin(115200);
  node.debug(&Serial);
  
  node.setup(pwd, false); //กรณี node ไม่ใช่ primary
  //node.setup(pwd, true); //กรณี node เป็น primary

//  Serial.println("------------------------------");
//  Serial.println("WiFi Status List");
//  Serial.print("WL_CONNECTED : ");   Serial.println(WL_CONNECTED);
//  Serial.print("WL_NO_SHIELD : ");   Serial.println(WL_NO_SHIELD); 
//  Serial.print("WL_IDLE_STATUS : ");   Serial.println(WL_IDLE_STATUS); 
//  Serial.print("WL_NO_SSID_AVAIL : ");   Serial.println(WL_NO_SSID_AVAIL); 
//  Serial.print("WL_SCAN_COMPLETED : ");   Serial.println(WL_SCAN_COMPLETED); 
//  Serial.print("WL_CONNECT_FAILED : ");   Serial.println(WL_CONNECT_FAILED); 
//  Serial.print("WL_CONNECTION_LOST : ");   Serial.println(WL_CONNECTION_LOST); 
//  Serial.print("WL_DISCONNECTED : ");   Serial.println(WL_DISCONNECTED); 
//  Serial.println("------------------------------");
//    
//  Serial.print("isPrimary? "); Serial.println(node.isPrimary());
}
int checker1=0;
int checker0=1;

void loop() {
  // put your main code here, to run repeatedly:
  node.loop();
  
  if(node.getStatus() == WL_CONNECTED) {
    if(checker1==0){
      Serial.print(WiFi.localIP());
      Serial.print(" >>>> Connect to >>>> ");
      Serial.println(WiFi.SSID());
      
      checker1=1;
      checker0=0;
    }
  }else {
    if(checker0==0){
      //Serial.println("0");
      checker1=0;
      checker0=1;
    }
  }

}