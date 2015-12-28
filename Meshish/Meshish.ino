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

  //node.setup(pwd, NODE_PRIMARY); //กรณี node เป็น Primary ... Port ปกติ คือ 3000
  //node.setup(pwd, NODE_PRIMARY, 3002); //กรณี node เป็น Primary .... กำหนด Port เอง
  node.setup(pwd, NODE_SECONDARY); //กรณี node เป็น Secondary

}


void loop() {
  // put your main code here, to run repeatedly:
  node.loop();
  
}
