#include "ESP8266WiFi.h"
#include "WiFiClient.h"

String ssid="testAP";
String pwd ="";

WiFiClient webclient;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(),pwd.c_str());
  Serial.println("\nWIFI begin :"+ssid+"\n");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nwifi connected!");
  Serial.print("Local IP : "); Serial.println(WiFi.localIP());
  
}

void loop() {
  // put your main code here, to run repeatedly:

  //ขอเชื่อมต่อไปยัง webserver ทาง port 80
  WiFiClient c;
  while(!c.connect("192.168.4.1",8080)){
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.print("\nconnected to the WEB-server\n");

  //ส่งคำร้องต้องการดู web site
  c.print(String("GET ") + "/" + " HTTP/1.1\r\n" + "Host: " + "192.168.4.1:8080" + "\r\n" + "Connection: close\r\n\r\n");
  delay(10);

  //รับค่าคืนตอบกลับมาแล้วแสดงทาง serial
  Serial.println();
  while(c.available()){
    String line = c.readStringUntil('\n');
    Serial.println(line);
  }

  Serial.println("\n------------------------------------------");  
  delay(5000);
}