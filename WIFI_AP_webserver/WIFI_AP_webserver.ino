#include ESP8266WiFi.h
#include ESP8266WebServer.h


String ssid=testAP;
String pwd =;
ESP8266WebServer server(8080);

void rootHandler(){
  server.send(200,texthtml,String(random(1,100))+n);
}

void testHandler(){
  server.send(200,texthtml,{{temp24}});
}

void setup() {
  Serial.begin(115200);
   put your setup code here, to run once
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid.c_str(), pwd.c_str());
  Serial.println(nWIFI begin);
  Serial.println(AP IP );
  Serial.println(WiFi.softAPIP());

  server.on(,rootHandler);
  server.on(test,testHandler);
  server.begin();
}

void loop() {
   put your main code here, to run repeatedly
  server.handleClient();
}
