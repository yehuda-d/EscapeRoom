// יהודה דייויס 208912949 - בקר חידות משולב סיימון ו-MUX
#include <ESP8266WiFi.h>
#include <DHT.h>

const char* ssid = "Reut";
const char* password = "rd357821";

// --- הגדרות MUX ---
const int MUX_A = D5;
const int MUX_B = D6;
const int MUX_C = D7;
const int MUX_IO = A0; 

const int LDR_PIN = A0;

// --- הגדרות סיימון ---
#define LED_COUNT 4
int leds[LED_COUNT] = {D1, D2, D3, D8}; 
int btnChannels[LED_COUNT] = {0, 1, 2, 4};

int lightSequence[8]; 
int userIndex = 0;
int showIndex = 0;
unsigned long simonTimer = 0;
bool isLedOn = false;
bool waitForRelease = false;
#define SIMON_SHOW 201
#define SIMON_PLAY 202

// --- הגדרות שאר החידות ---
const int MOTOR_A = D2;      
const int MOTOR_B = D1;      
const int DHT_PIN = D4;      
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

int puzzleState = 0; 
int maxLightValue = analogRead(LDR_PIN);
float initialTemp = 0;
unsigned long lightLowStartTime = 0;
bool timingStarted = false;

void setup_ClientWiFi();
bool sendUpdateToServer(int step);

// פונקציה לבחירת ערוץ ב-MUX וקריאת ערך
int readMux(int channel) {
  digitalWrite(MUX_A, channel & 0x01);
  digitalWrite(MUX_B, (channel >> 1) & 0x01);
  digitalWrite(MUX_C, (channel >> 2) & 0x01);
  delay(10); // זמן התייצבות קצר
  return analogRead(MUX_IO);
}

void setup() {
  Serial.begin(115200);
  
  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);
  
  pinMode(MOTOR_A, OUTPUT); pinMode(MOTOR_B, OUTPUT);
  digitalWrite(MOTOR_A, LOW); digitalWrite(MOTOR_B, LOW);
  
  for (int i = 0; i < LED_COUNT; i++) {
    pinMode(leds[i], OUTPUT);
  }
  
  dht.begin();
  setup_ClientWiFi(); 
  //randomSeed(readMux(4)); // שימוש ברעש מה-LDR לאתחול רנדומלי

  Serial.println("\n--- Calibrating... ---");
  delay(2000); 
  initialTemp = dht.readTemperature();
  
  Serial.print("Initial Light: "); Serial.println(maxLightValue);
  Serial.print("Initial Temp: "); Serial.println(initialTemp);
}

void loop() {
  int currentLight = analogRead(LDR_PIN);
  float targetLight = maxLightValue * 0.8;

  switch (puzzleState) {
    
    case 0: // חידת האור
      if (currentLight < targetLight) {
        // אם האור נמוך והטיימר עדיין לא פועל - נתחיל לספור
        if (!timingStarted) {
          lightLowStartTime = millis(); // שומר את הזמן הנוכחי
          timingStarted = true;
          Serial.println("Light dimmed... ");
        }

        // חישוב: כמה זמן עבר מאז שהאור ירד?
        unsigned long duration = millis() - lightLowStartTime;
        Serial.print("[LIGHT LOW] Time: "); 
        Serial.println(duration / 1000.0); // מציג שניות

        // אם עברו 2 שניות (2000 מילישניות)
        if (duration >= 2000) {
          Serial.println("\n>>> Light held low for 2 seconds! Step 1 Solved! <<<");
          sendUpdateToServer(1); 
          
          digitalWrite(MOTOR_A, HIGH);
          digitalWrite(MOTOR_B, LOW);
          
          puzzleState = 1;
          timingStarted = false; // איפוס הטיימר
          Serial.println("Fan ON. Moving to Temperature Puzzle...\n");
        }
      } 
      else {
        // אם האור חזר להיות חזק לפני שעברו 2 שניות - מאפסים את הספירה
        if (timingStarted) {
          Serial.println("Light returned too early! Timer reset.");
          timingStarted = false;
        }
        
        Serial.print("[LIGHT MODE] Current: "); 
        Serial.print(currentLight);
        Serial.print(" | Target: < "); 
        Serial.println(targetLight);
      }
      break;

    case 1: // חידה 2: טמפרטורה
      {
        float currentTemp = dht.readTemperature();
        float targetTemp = initialTemp - 2.0;
        
        if (!isnan(currentTemp)) {
          Serial.print("Temp: "); Serial.print(currentTemp);
          Serial.print("C | Target: <= "); Serial.print(targetTemp); Serial.println("C");
          
          if (currentTemp <= targetTemp) {
            sendUpdateToServer(2);
            digitalWrite(MOTOR_A, LOW); digitalWrite(MOTOR_B, LOW);
            puzzleState = 2; 
            generateSimonSequence();
            Serial.println(">>> Step 2 Solved! Starting Simon...");
            puzzleState = SIMON_SHOW;
            simonTimer = millis();
          }
        }
      }
      break;

    case SIMON_SHOW:
      showSimonSequence();
      break;

    case SIMON_PLAY:
      checkUserSimon();
      break;

    case 3:
      break;
  }
  
  if (puzzleState < 2) delay(500); 
  else delay(10); 
}

void generateSimonSequence() {
  for (int i = 0; i < 8; i++) { lightSequence[i] = random(4); }
}

void showSimonSequence() {
  if (showIndex >= 8) {
    puzzleState = SIMON_PLAY;
    showIndex = 0;
    userIndex = 0;
    return;
  }
  if (!isLedOn && millis() - simonTimer >= 500) {
    digitalWrite(leds[lightSequence[showIndex]], HIGH);
    isLedOn = true;
    simonTimer = millis();
  }
  if (isLedOn && millis() - simonTimer >= 500) {
    digitalWrite(leds[lightSequence[showIndex]], LOW);
    isLedOn = false;
    showIndex++;
    simonTimer = millis();
  }
}

void checkUserSimon() {
  int pressed = -1;
  // סריקה של ערוצים 0 עד 3 ב-MUX כדי למצוא לחיצה
  for (int i = 0; i < 4; i++) {
    // אם הערך נמוך מ-200, סימן שהכפתור לחוץ (מחובר ל-GND)
    if (readMux(btnChannels[i]) < 200) { 
      pressed = i; 
      break; 
    }
  }

  if (pressed == -1) { waitForRelease = false; return; }

  if (!waitForRelease) {
    waitForRelease = true;
    digitalWrite(leds[pressed], HIGH); 
    delay(200);
    digitalWrite(leds[pressed], LOW);
    
    if (pressed == lightSequence[userIndex]) {
      Serial.println("Simon: Correct!");
      userIndex++;
      if (userIndex >= 8) {
        Serial.println(">>> Simon Solved! going to puzzle 4");
        sendUpdateToServer(3);
        puzzleState = 3;
      }
    } else {
      Serial.println("Simon: Wrong! Restarting...");
      userIndex = 0; showIndex = 0;
      puzzleState = SIMON_SHOW;
      delay(1000);
    }
  }
}