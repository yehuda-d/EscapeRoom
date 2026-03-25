
// יהודה דייויס 208912949 | אריאל חזוט 324056670 - בקר חידות  
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <NewPing.h>

const char* ssid = "EscapeRoom_Lock";
const char* password = "666";

// --- הגדרות MUX ---
const int MUX_A = D5;
const int MUX_B = D6;
const int MUX_C = D7;
const int MUX_IO = A0;

const int LDR_PIN = A0;


#define TRIGGER_PIN D5  
#define ECHO_PIN D6
#define MAX_DISTANCE 100 
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
unsigned long distStartTime = 0;
bool distTimingStarted = false;

// --- הגדרות סיימון ---
#define LED_COUNT 4
int leds[LED_COUNT] = { D1, D2, D3, D8 };
int btnChannels[LED_COUNT] = { 0, 1, 2, 4 };

int lightSequence[8];
int userIndex = 0;
int showIndex = 0;
unsigned long simonTimer = 0;
bool isLedOn = false;
bool waitForRelease = false;
#define SIMON_SHOW 201
#define SIMON_PLAY 202


const int MOTOR_A = D2;
const int MOTOR_B = D1;
const int DHT_PIN = D4;
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

int puzzleState = 0; 
int maxLightValue = 0; 
float initialTemp = 0;
unsigned long lightLowStartTime = 0;
bool timingStarted = false;

void setup_ClientWiFi();
bool sendUpdateToServer(int step);

int readMux(int channel) {
  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  digitalWrite(MUX_A, channel & 0x01);
  digitalWrite(MUX_B, (channel >> 1) & 0x01);
  digitalWrite(MUX_C, (channel >> 2) & 0x01);
  delay(10);
  return analogRead(MUX_IO);
}

void setup() {
  Serial.begin(9600);

  pinMode(MUX_A, OUTPUT);
  pinMode(MUX_B, OUTPUT);
  pinMode(MUX_C, OUTPUT);

  pinMode(MOTOR_A, OUTPUT);
  pinMode(MOTOR_B, OUTPUT);
  digitalWrite(MOTOR_A, LOW);
  digitalWrite(MOTOR_B, LOW);

  for (int i = 0; i < LED_COUNT; i++) {
    pinMode(leds[i], OUTPUT);
  }

  dht.begin();
   setup_ClientWiFi(); 

  Serial.println("\n--- Calibrating... ---");
  delay(2000);
  maxLightValue = analogRead(LDR_PIN);
  initialTemp = dht.readTemperature();
}

void loop() {
  int currentLight = analogRead(LDR_PIN);
  float targetLight = maxLightValue * 0.8;

  switch (puzzleState) {

    case 0:  // חידה 1: אור
      {
        Serial.print("LDR: "); Serial.println(currentLight);
        if (currentLight < targetLight) {
          if (!timingStarted) { lightLowStartTime = millis(); timingStarted = true; }
          if (millis() - lightLowStartTime >= 2000) {
            sendUpdateToServer(1);
            digitalWrite(MOTOR_A, HIGH);
            Serial.println("moving to puzzle 2");
            puzzleState = 1; timingStarted = false;
          }
        } else { timingStarted = false; }
      }
      break;

    case 1:  // חידה 2: טמפרטורה
      {
        float currentTemp = dht.readTemperature();
        Serial.print("Temp: "); Serial.println(currentTemp);
        if (!isnan(currentTemp) && currentTemp <= (initialTemp - 2.0)) {
           sendUpdateToServer(2);
          digitalWrite(MOTOR_A, LOW);
          Serial.println("moving to puzzle 3");
          generateSimonSequence();
          puzzleState = SIMON_SHOW;
        }
      }
      break;

//חידה 3 סיימון
    case SIMON_SHOW: showSimonSequence(); break;
    case SIMON_PLAY: checkUserSimon(); break;

    case 3: // חידה 4: מרחק
      {
        pinMode(TRIGGER_PIN, OUTPUT);
        pinMode(ECHO_PIN, INPUT);
        
        int distance = sonar.ping_cm();
        Serial.print("Distance: ");
        Serial.print(distance);
        Serial.println("cm");

        if (distance >= 18 && distance <= 22) {
          if (!distTimingStarted) {
            distStartTime = millis();
            distTimingStarted = true;
            Serial.println("Distance 20cm detected! Hold it...");
          }
          if (millis() - distStartTime >= 2000) {
             sendUpdateToServer(4);
            Serial.println(">>> PUZZLE 4 SOLVED! You finished all the puzzles!!! <<<");
            puzzleState = 4;
          }
        } else {
          distTimingStarted = false;
        }
        delay(60); // מנוחה לחיישן
      }
      break;
      
    case 4: break;
  }

  if (puzzleState < 2) delay(500);
  else delay(10);
}

// פונקציות סיימון
void generateSimonSequence() { for (int i = 0; i < 8; i++) { lightSequence[i] = random(4); } }

void showSimonSequence() {
  if (showIndex >= 8) { puzzleState = SIMON_PLAY; showIndex = 0; userIndex = 0; return; }
  if (!isLedOn && millis() - simonTimer >= 500) {
    digitalWrite(leds[lightSequence[showIndex]], HIGH);
    isLedOn = true; simonTimer = millis();
  }
  if (isLedOn && millis() - simonTimer >= 500) {
    digitalWrite(leds[lightSequence[showIndex]], LOW);
    isLedOn = false; showIndex++; simonTimer = millis();
  }
}

void checkUserSimon() {
  int pressed = -1;
  for (int i = 0; i < 4; i++) {
    if (readMux(btnChannels[i]) < 200) { pressed = i; break; }
  }
  if (pressed == -1) { waitForRelease = false; return; }
  if (!waitForRelease) {
    waitForRelease = true;
    digitalWrite(leds[pressed], HIGH);
    delay(200);
    digitalWrite(leds[pressed], LOW);
    if (pressed == lightSequence[userIndex]) {
      userIndex++;
      if (userIndex >= 8) {
        Serial.println("moving to puzzle 4");
         sendUpdateToServer(3);
         puzzleState = 3; 
         }
    } else {
      Serial.println("wrong, try again");
      userIndex = 0; showIndex = 0; puzzleState = SIMON_SHOW; delay(1000);
    }
  }
}











