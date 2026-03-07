// #include <ESP8266WiFi.h>
// #include <WiFiClient.h>
// #include <WiFiUdp.h>
// #include <ESP8266HTTPClient.h>
// #include <DHT.h>

// // טאב ניהול WiFi ותקשורת

// void wifi_Setup() {
//   Serial.print("Connecting to ");
//   Serial.println(ssid);
//   WiFi.begin(ssid, password);
  
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("\nWiFi connected");
//   Serial.print("IP Address: ");
//   Serial.println(WiFi.localIP());
// }

// void sendUpdateToLock(int step) {
//   // אם ה-WiFi התנתק, ננסה לחבר אותו מחדש
//   if (WiFi.status() != WL_CONNECTED) {
//     Serial.println("WiFi connection lost. Reconnecting...");
//     WiFi.begin(ssid, password);
    
//     // נחכה עד 10 שניות לחיבור מחדש
//     int retryCount = 0;
//     while (WiFi.status() != WL_CONNECTED && retryCount < 20) {
//       delay(500);
//       Serial.print(".");
//       retryCount++;
//     }
//     Serial.println("");
//   }

//   // עכשיו נבדוק שוב אם אנחנו מחוברים
//   if (WiFi.status() == WL_CONNECTED) {
//     WiFiClient client;
//     HTTPClient http;
    
//     String url = "http://" + String(lockIP) + "/update?step=" + String(step);
//     Serial.print("Sending to: "); Serial.println(url);
    
//     http.begin(client, url);
//     int httpCode = http.GET();
    
//     if (httpCode > 0) {
//       Serial.print("Success! Response: "); Serial.println(httpCode);
//     } else {
//       Serial.print("Failed to send, error: "); Serial.println(http.errorToString(httpCode).c_str());
//     }
//     http.end();
//   } else {
//     Serial.println("Error: Still no WiFi connection. Check your router.");
//   }
// }






// // הצהרה על פונקציות ומשתנים שנמצאים בטאב הראשי
// extern const char* ssid;
// extern const char* password;
// void updateGameState(int stepReceived); // הצהרה על הפונקציה מהטאב הראשי

// void setup_Server() {
//   WiFi.begin(ssid, password);
//   Serial.print("Connecting to WiFi");
  
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//     // בגלל שאין לנו אובייקט display כאן, נוריד את ה-display.loop מה-WiFi
//   }
  
//   Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

//   server.on("/update", HTTP_GET, []() {
//     if (server.hasArg("step")) {
//       int stepNum = server.arg("step").toInt();
      
//       // במקום לשנות משתנים כאן, אנחנו קוראים לפונקציה בטאב הראשי
//       updateGameState(stepNum);
      
//       server.send(200, "text/plain", "OK. State updated to step " + String(stepNum));
//     } else {
//       server.send(400, "text/plain", "Missing step argument");
//     }
//   });

//   server.begin();
// }



#include <ESP8266HTTPClient.h>

String serverIP = "http://192.168.1.13"; 

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
    
    return (httpCode > 0); // מחזיר אמת אם השרת ענה
  }
  return false;
}

 
