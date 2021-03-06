#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "MeshishNode.h"


// Password ของทั้งระบบ node ทุกตัวตั้งค่าเดียวกันที่นี่
const char* pwd = "";

MeshishNode node;

struct AccessPoint {
    String ssid;
    long rssi;
    byte encrypt;
    unsigned int nodeType;
  };

void setup() {
  Serial.begin(115200);
  //node.debug(&Serial);

  //node.setup(pwd, true);
}

void loop() {
  // put your main code here, to run repeatedly:
  //node.loop();

  Serial.println("scan start");
  int numNetworks = (WiFi.scanNetworks() < MESHISH_MAX_AP_SCAN)? WiFi.scanNetworks(): MESHISH_MAX_AP_SCAN;
  AccessPoint apList[MESHISH_MAX_AP_SCAN];
  Serial.println("scan done");
  
  // the index of the node with the highest rssi
  int maxDBmPrimary = -1;

  Serial.print(numNetworks); Serial.println(" AP networks found");
  
  for (int i = 0; i < numNetworks; i++)
  {
    apList[i] = AccessPoint();
    apList[i].ssid     = WiFi.SSID(i);             // ชื่อของ Access Point 
    apList[i].rssi     = WiFi.RSSI(i);             // ความแรงของสัญญาณของ AP (หน่วยคือ dBm)
    apList[i].encrypt  = WiFi.encryptionType(i);   // 
    //apList[i].nodeType = _getNodeType(apList[i]); // เป็น Primary หรือ Secondary Node?

    Serial.print(i+1); Serial.print(": "); Serial.print(apList[i].ssid); Serial.print(" ("); Serial.print(apList[i].rssi); Serial.print(" dBm)");
    Serial.println((apList[i].encrypt == ENC_TYPE_NONE)?" ":"*");


    if (maxDBmPrimary == -1)
    {
      maxDBmPrimary = i;
    }
    else
    {
      if (apList[i].rssi > apList[maxDBmPrimary].rssi)
      {
        maxDBmPrimary = i;                       // index ของ AP จากที่เจอทั้งหมด ที่ มีสัญญาณแรงสุดๆ
      }
    }
  }
  Serial.println();Serial.println();Serial.println();
  
  if (maxDBmPrimary != -1)
  {    
    Serial.print("1st wifi: ");Serial.print(apList[maxDBmPrimary].ssid.c_str());Serial.print(" (");Serial.print(apList[maxDBmPrimary].rssi);Serial.print(" dBm)");
    Serial.println((apList[maxDBmPrimary].encrypt == ENC_TYPE_NONE)?" ":"*");

    WiFi.begin(WiFi.SSID(maxDBmPrimary).c_str(), ""); 
    int status = WiFi.status();
    IPAddress gateway_ip, local_ip;

    // wait for connection or fail
    while(status != WL_CONNECTED && status != WL_NO_SSID_AVAIL && status != WL_CONNECT_FAILED) {
        delay(10);
        status = WiFi.status();
    }

    
    switch(status) {
        case WL_CONNECTED:
            local_ip = WiFi.localIP();
            gateway_ip = WiFi.gatewayIP();

            Serial.println("[WIFI] Connecting done.");
            Serial.print("[WIFI] SSID: "); Serial.println(WiFi.SSID());
            Serial.print("[WIFI] IP: "); Serial.print(local_ip); Serial.print(" [Gateway: ");Serial.print(gateway_ip); Serial.println(" ]");
            break;
        case WL_NO_SSID_AVAIL:
            Serial.println("[WIFI] Connecting Faild AP not found.");
            break;
        case WL_CONNECT_FAILED:
            Serial.println("[WIFI] Connecting Faild.");
            break;
        default:
            Serial.print("[WIFI] Connecting Faild "); Serial.println(status);
            break;
    }
    
    Serial.println();Serial.println("----------------------------------");
  
  }
  Serial.println("");

  delay(5000);
  WiFi.disconnect();
}
