
#include <ESP8266HTTPClient.h>

String serverIP = "http://192.168.4.1"; 

extern const char* ssid;
extern const char* password;

void setup_ClientWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
}

bool sendUpdateToServer(int step) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    String url = serverIP + "/update?step=" + String(step);
    
    http.begin(client, url);
    int httpCode = http.GET();
    http.end();
    
    return (httpCode > 0); 
  }
  return false;
}

 


