/*
 */

#include <ESP8266WiFi.h>

const char *ssid = "MyESPAP";
const char *password = "";
const char *host = "192.168.4.1";

//const char *ssid = "ANJOLEENA";
//const char *password = "anjoleena";
//const char *host = "192.168.1.16";


int rate_toogle = 0;


void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to WIFI :");
  Serial.println(ssid); 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() {
  if (rate_toogle == 0)
    rate_toogle = 1;
  else
    rate_toogle = 0;
  Serial.print("connecting to ");
  Serial.println(host);
  WiFiClient client;
  const int httpPort = 80;
  while (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    delay(2000);   
  }
  
  Serial.println("Connected!!!");
  String url = "/frate";
  if (rate_toogle == 0)
    url = "/srate";  

  Serial.print("Requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  delay(10);
  while(client.available()){
    String line = client.readStringUntil('\n');
    Serial.print(line);
  }

  Serial.println("\n\nclosing connection");
  delay(10000);
  Serial.println("------------------------------------------");
}


