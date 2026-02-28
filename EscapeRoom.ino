#include <ESP8266WiFi.h>

#define LDR_PIN A0

int maxLightValue = 0; 
int lightThreshold = 0; 
unsigned long lightStartTime = 0; 
bool isDimmed = false;
int currentPuzzle = 1;

void setup() {
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT);

  // --- תיקון 1: כיול ---
  Serial.println("Calibrating light in 2 seconds... Keep lights ON");
  delay(2000); 
  maxLightValue = analogRead(LDR_PIN); // מודד את האור המקסימלי בחדר
  lightThreshold = maxLightValue * 0.8; // קובע סף של 20% פחות
  
  Serial.print("Max Light: "); Serial.println(maxLightValue);
  Serial.print("Threshold set to: "); Serial.println(lightThreshold);
  
  // wifi_Setup(); // ודא שהפונקציה הזו קיימת אצלך בקוד
}

void loop() {
  switch (currentPuzzle) {
    case 1:
      handlePuzzle1();
      break;
    case 2:
      // כאן תבוא חידה 2
      break;
  }
}

void handlePuzzle1() {
  int currentLight = analogRead(LDR_PIN);

  if (currentLight <= lightThreshold) {
    if (!isDimmed) {
      lightStartTime = millis();
      isDimmed = true;
      Serial.println("Light dimmed, starting timer...");
    }
    
    if (millis() - lightStartTime >= 2000) {
      Serial.println("Puzzle 1 Solved!");
      sendUpdateToLock(1); 
      
      // --- תיקון 2: שימוש בשם המשתנה הנכון ---
      currentPuzzle = 2; 
    }
  } 
  else {
    if (isDimmed) {
      Serial.println("Light too bright! Timer reset.");
      isDimmed = false;
    }
  }
}

void sendUpdateToLock(int step) {
  Serial.print("Sending update to Lock Controller: Step ");
  Serial.println(step);
  // כאן יבוא קוד ה-HTTP בהמשך
}