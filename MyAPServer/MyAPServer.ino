/*
 */


#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>

const char *ssid = "MyESPAP";
const char *password = "";
//const char *ssid = "ANJOLEENA";
//const char *password = "anjoleena";


ESP8266WebServer server(80);

int led = D4;
int blink_count = 0;
int blink_rate = 250;

void handleRoot() {
  server.send(200, "text/html", "<html><body><h1>MyESPAP Home Page</h1>\r\n<a href=\"/frate\">Fast Rate</a><br>\r\n<a href=\"/srate\">Slow Rate</a><br></body></html>");
}

void handleFRate() {
  blink_rate = 125;
  Serial.println("FastRate mode !");
  server.send(200, "text/html", "<html><head><script>window.onload =  function() { setInterval(function() {window.location.replace('/');}, 1000); };</script></head><body><h1>Fast Rate mode</h1></body></html>");
}

void handleSRate() {
  blink_rate = 500;
  Serial.println("SlowRate mode !");
  server.send(200, "text/html", "<html><head><script>window.onload =  function() { setInterval(function() {window.location.replace('/');}, 1000); };</script></head><body><h1>Slow Rate mode</h1></body></html>");
}

void setup() {
  pinMode(led, OUTPUT);
  delay(1000);
  Serial.begin(115200);
  
  Serial.println();

  Serial.println("Configuring Access Point ...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());


//    WiFi.mode(WIFI_STA);
//    Serial.print("\nConnecting to WIFI :");
//    Serial.println(ssid); 
//    WiFi.begin(ssid,password);
//    while(WiFi.status() != WL_CONNECTED){
//      Serial.print(".");
//      delay(500);      
//    }
//    Serial.println("\nWIFI connected");
//    Serial.println("local IP : "+WiFi.localIP().toString());
  
  server.on("/", handleRoot);
  server.on("/frate", handleFRate);
  server.on("/srate", handleSRate);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  if (blink_count >= blink_rate) {
    digitalWrite(led, HIGH);
    blink_count = 0;
  }
  if (blink_count == blink_rate / 2) {
    digitalWrite(led, LOW);
  }
  delay(1);
  blink_count++;
}


