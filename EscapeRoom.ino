// יהודה דייויס 208912949 - בקר חידות (אור וטמפרטורה)
#include <ESP8266WiFi.h>
#include <DHT.h>

const char* ssid = "Reut";
const char* password = "rd357821";

const int LDR_PIN = A0;      
const int MOTOR_A = D2;      
const int MOTOR_B = D1;      
const int DHT_PIN = D4;      
#define DHTTYPE DHT22

DHT dht(DHT_PIN, DHTTYPE);

int puzzleState = 0; 
int maxLightValue = 0;
float initialTemp = 0;

// הצהרות על פונקציות מהטאב השני
void setup_ClientWiFi();
bool sendUpdateToServer(int step);

unsigned long lightLowStartTime = 0; // משתנה למדידת זמן
bool timingStarted = false;          // האם התחלנו לספור?

void setup() {
  Serial.begin(115200);
  
  pinMode(MOTOR_A, OUTPUT);
  pinMode(MOTOR_B, OUTPUT);
  digitalWrite(MOTOR_A, LOW);
  digitalWrite(MOTOR_B, LOW);
  
  dht.begin();
  setup_ClientWiFi(); 

  Serial.println("\n--- Calibrating Sensors (Please wait...) ---");
  delay(2000); 
  
  maxLightValue = analogRead(LDR_PIN);
  initialTemp = dht.readTemperature();
  
  Serial.print("Initial Light (Max): "); Serial.println(maxLightValue);
  Serial.print("Initial Temp: "); Serial.println(initialTemp);
  Serial.println("System Ready!\n");
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
          Serial.println("Light dimmed... Keep it low for 2 seconds!");
        }

        // חישוב: כמה זמן עבר מאז שהאור ירד?
        unsigned long duration = millis() - lightLowStartTime;
        Serial.print("[LIGHT LOW] Time: "); 
        Serial.print(duration / 1000.0); // מציג שניות
        Serial.println("s / 2.0s");

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

    case 1: // חידת הטמפרטורה
      {
        float currentTemp = dht.readTemperature();
        float targetTemp = initialTemp - 2.0; 

        if (!isnan(currentTemp)) {
          Serial.print("[TEMP MODE] Current: "); 
          Serial.print(currentTemp);
          Serial.print("C | Target: <= "); 
          Serial.print(targetTemp);
          Serial.println("C");

          if (currentTemp <= targetTemp) {
            Serial.println("\n>>> Temp Dropped! Step 2 Solved! <<<");
            sendUpdateToServer(2); 
            
            digitalWrite(MOTOR_A, LOW);
            digitalWrite(MOTOR_B, LOW);
            
            Serial.println("Moving to the next puzzle...\n");
            
            puzzleState = 2; // עובר למצב הבא (חידה שלישית בעתיד)
          }
        } else {
          Serial.println("Error reading Temp sensor!");
        }
      }
      break;

    case 2:
      // כאן תבוא הלוגיקה של החידה השלישית
      break;
  } // סגירת ה-switch

  delay(800); 
} // סגירת ה-loop