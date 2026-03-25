// יהודה דייויס 208912949 | אריאל חזוט 324056670 - בקר הנעילה  

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


const char* ssid = "EscapeRoom_Lock"; // שם הרשת שהבקר ישדר
const char* password = "666"; // הסיסמה לרשת

ESP8266WebServer server(80);

// --- הגדרות חומרה ---
const int RELAY_PIN = D1;  
const int CLK_PIN = D5;    
const int LATCH_PIN = D6;  
const int DATA_PIN = D7;   

#define RELAY_ON LOW
#define RELAY_OFF HIGH

const String SECRET_CODE = "7392";
int puzzlesSolved = 0;

// --- תצוגה ---
const byte digitToSegment[10] = {
  0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F
};
const byte OFF_SEGMENT = 0x00; 
const byte digitSelect[4] = {0x07, 0x0B, 0x0D, 0x0E}; 

void refreshDisplay() {
  byte currentData[4] = {0x00, 0x00, 0x00, 0x00};

  if (puzzlesSolved >= 1) currentData[0] = digitToSegment[SECRET_CODE[0] - '0'];
  if (puzzlesSolved >= 2) currentData[1] = digitToSegment[SECRET_CODE[1] - '0'];
  if (puzzlesSolved >= 3) currentData[2] = digitToSegment[SECRET_CODE[2] - '0'];
  if (puzzlesSolved >= 4) currentData[3] = digitToSegment[SECRET_CODE[3] - '0'];

  for (int i = 0; i < 4; i++) {
    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, 0x00);
    shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, 0x0F);
    digitalWrite(LATCH_PIN, HIGH);

    digitalWrite(LATCH_PIN, LOW);
    shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, currentData[i]); 
    shiftOut(DATA_PIN, CLK_PIN, MSBFIRST, digitSelect[i]); 
    digitalWrite(LATCH_PIN, HIGH);
    
    delay(3);
  }
}

// --- דפי אינטרנט ---
const char htmlIndex[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html dir='rtl'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>חדר בריחה</title><style>body { background-color: #1a1a2e; color: #fff; font-family: 'Segoe UI', Tahoma, sans-serif; display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; margin: 0; } .box { background: #16213e; padding: 40px; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.5); text-align: center; border: 1px solid #0f3460; } h1 { color: #e94560; font-size: 2.2em; margin-bottom: 10px; } input { font-size: 30px; padding: 15px; width: 180px; text-align: center; margin: 20px 0; letter-spacing: 8px; border: 2px solid #e94560; border-radius: 8px; background: #1a1a2e; color: #fff; outline: none; } input:focus { box-shadow: 0 0 15px rgba(233, 69, 96, 0.5); } button { font-size: 22px; padding: 12px 30px; cursor: pointer; background-color: #e94560; color: white; border: none; border-radius: 8px; font-weight: bold; transition: 0.3s; } button:hover { background-color: #d13b56; transform: translateY(-2px); }</style></head><body><div class='box'><h1>מערכת נעילה ראשית</h1><p>הזינו את הקוד הסודי שהתקבל:</p><form action='/unlock' method='POST'><input type='text' name='code' maxlength='4' autocomplete='off' required autofocus><br><button type='submit'>פתח דלת</button></form></div></body></html>
)rawliteral";

const char htmlVictory[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html dir='rtl'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>ניצחון!</title><style>body { background-color: #0d1117; color: #fff; font-family: 'Segoe UI', Tahoma, sans-serif; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; } .container { text-align: center; background: #161b22; padding: 50px; border-radius: 20px; box-shadow: 0 0 30px rgba(46, 160, 67, 0.4); border: 2px solid #2ea043; max-width: 450px; width: 90%; } h1 { color: #3fb950; font-size: 3em; margin-bottom: 15px; text-shadow: 0 0 15px rgba(63, 185, 80, 0.5); } p { font-size: 1.3em; color: #c9d1d9; line-height: 1.6; } .icon { font-size: 5em; margin-bottom: 20px; animation: bounce 2s infinite; } @keyframes bounce { 0%, 20%, 50%, 80%, 100% {transform: translateY(0);} 40% {transform: translateY(-20px);} 60% {transform: translateY(-10px);} }</style></head><body><div class='container'><div class='icon'>🔓🎉</div><h1>כל הכבוד!</h1><p>הצלחתם לפתור את כל החידות, לפרוץ את הקוד ולברוח מהחדר!</p></div></body></html>
)rawliteral";

void handleRoot() { server.send(200, "text/html", htmlIndex); }

void handleUnlock() {
  if (server.hasArg("code")) {
    String enteredCode = server.arg("code");
    if (enteredCode == SECRET_CODE && puzzlesSolved == 4) {
      Serial.println("!!! PASSWORD CORRECT - UNLOCKING DOOR !!!");
      digitalWrite(RELAY_PIN, RELAY_ON); 
      server.send(200, "text/html", htmlVictory);
      delay(5000); 
      digitalWrite(RELAY_PIN, RELAY_OFF); 
    } else {
      Serial.println("Wrong password or puzzles not finished.");
      server.send(200, "text/html", "<html dir='rtl'><body style='background-color:#1a1a2e; color:white; text-align:center; font-family:Arial; padding-top:100px;'><h1 style='color:#e94560;'>קוד שגוי או החידות טרם הושלמו 🔒</h1><br><a href='/' style='color:#ccc; font-size:20px; text-decoration:none; border-bottom:1px solid #ccc;'>חזור לנסות שוב</a></body></html>");
    }
  } else { server.send(400, "text/plain", "Bad Request"); }
}

void handleUpdate() {
  if (server.hasArg("step")) {
    int step = server.arg("step").toInt();
    if (step >= 1 && step <= 4) {
      puzzlesSolved = step;
      
     
      Serial.print(">>> YEHUDA SOLVED PUZZLE NUMBER: ");
      Serial.println(step);
      
      server.send(200, "text/plain", "OK");
    } else { server.send(400, "text/plain", "Invalid step"); }
  } else { server.send(400, "text/plain", "Missing step"); }
}

void setup() {
  Serial.begin(9600);
  
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(CLK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, RELAY_OFF); 


  Serial.println("\nStarting Access Point...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  Serial.print("Access Point Started! Network Name: ");
  Serial.println(ssid);
  Serial.print("My IP address is always: ");
  Serial.println(WiFi.softAPIP()); // תמיד יהיה 192.168.4.1

  server.on("/", HTTP_GET, handleRoot);
  server.on("/unlock", HTTP_POST, handleUnlock);
  server.on("/update", HTTP_GET, handleUpdate);
  server.begin();
}

void loop() {
  server.handleClient(); 
  refreshDisplay();      
}