#include <SoftwareSerial.h>

#define MQ9_PIN A0
#define BUZZER_PIN 8
#define SIM800_TX_PIN 10
#define SIM800_RX_PIN 11
#define X_PIN A1
#define Y_PIN A2
#define Z_PIN A3

#define GAS_THRESHOLD 500
#define MOVEMENT_THRESHOLD 100
#define SMS_COOLDOWN 60000
#define BUZZER_DURATION 60000

SoftwareSerial sim800l(SIM800_TX_PIN, SIM800_RX_PIN);

unsigned long lastSmsTime = 0;
unsigned long buzzerStartTime = 0;
bool buzzerActive = false;
bool simInitialized = false;

const int BASELINE_X = 512;
const int BASELINE_Y = 512;
const int BASELINE_Z = 512;

void setup() {
  Serial.begin(115200);
  Serial.println("System Initializing...");

  sim800l.begin(9600);
  pinMode(MQ9_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(X_PIN, INPUT);
  pinMode(Y_PIN, INPUT);
  pinMode(Z_PIN, INPUT);

  if (initializeSIM800()) {
    simInitialized = true;
    Serial.println("SIM800L Initialized.");
  } else {
    Serial.println("SIM800L Initialization Failed!");
  }
}

void loop() {
  if (!simInitialized) {
    return;
  }

  int gasLevel = analogRead(MQ9_PIN);
  int x = analogRead(X_PIN);
  int y = analogRead(Y_PIN);
  int z = analogRead(Z_PIN);

  if (gasLevel > GAS_THRESHOLD || detectMovement(x, y, z)) {
    Serial.println("Triggering Alert...");
    triggerAlert();
  }

  if (buzzerActive && millis() - buzzerStartTime >= BUZZER_DURATION) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerActive = false;
    Serial.println("Buzzer Deactivated.");
  }

  static unsigned long lastCheckTime = 0;
  if (millis() - lastCheckTime >= 1000) {
    lastCheckTime = millis();
    Serial.println("Monitoring...");
  }
}

bool detectMovement(int x, int y, int z) {
  return (abs(x - BASELINE_X) > MOVEMENT_THRESHOLD || abs(y - BASELINE_Y) > MOVEMENT_THRESHOLD || abs(z - BASELINE_Z) > MOVEMENT_THRESHOLD);
}

void triggerAlert() {
  if (!buzzerActive) {
    digitalWrite(BUZZER_PIN, HIGH);
    buzzerActive = true;
    buzzerStartTime = millis();
    Serial.println("Buzzer Activated.");
  }

  if (millis() - lastSmsTime >= SMS_COOLDOWN) {
    sendSMS("+639208771890", "Alert! Gas or Movement detected!");
    lastSmsTime = millis();
  }
}

void sendSMS(String number, String text) {
  Serial.println("Sending SMS...");
  sim800l.print("AT+CMGS=\"" + number + "\"\r");
  delay(100);
  sim800l.print(text);
  delay(100);
  sim800l.write(26);
  Serial.println("SMS Sent!");
}

bool initializeSIM800() {
  sendAT("AT");
  sendAT("AT+CMGF=1");
  sendAT("AT+CSCS=\"GSM\"");
  return true;
}

void sendAT(String cmd) {
  sim800l.println(cmd);
  delay(1000);
}
