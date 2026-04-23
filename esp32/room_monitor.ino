#include <WiFi.h>
#include <HTTPClient.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

// ============================================================
// WiFi Settings - CHANGE THESE
// ============================================================
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// ============================================================
// Server Settings - CHANGE THIS to your Railway URL
// ============================================================
const char* SERVER_URL = "https://YOUR_APP.up.railway.app/api/status";

// ============================================================
// LCD (I2C) - Address 0x27, 16 cols x 2 rows
// SDA -> GPIO21, SCL -> GPIO22
// ============================================================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ============================================================
// Keypad 4x4
// ============================================================
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// ESP32 GPIO pins for keypad rows and columns
// Adjust these to match your wiring
byte rowPins[ROWS] = {13, 12, 14, 27};
byte colPins[COLS] = {26, 25, 33, 32};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ============================================================
// State
// ============================================================
enum State {
  IDLE,
  INPUT_MINUTES
};

State currentState = IDLE;
bool isInRoom = true;        // true = in_room, false = out
String minutesInput = "";    // Buffer for digit input

// ============================================================
// Setup
// ============================================================
void setup() {
  Serial.begin(115200);

  // Init LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");

  // Connect WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.println(WiFi.localIP());
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
  } else {
    Serial.println("\nWiFi FAILED");
    lcd.setCursor(0, 0);
    lcd.print("WiFi FAILED!");
    lcd.setCursor(0, 1);
    lcd.print("Check settings");
  }

  delay(2000);
  showIdleScreen();
}

// ============================================================
// Main Loop
// ============================================================
void loop() {
  char key = keypad.getKey();

  if (key == NO_KEY) return;

  Serial.print("Key: ");
  Serial.println(key);

  switch (currentState) {
    case IDLE:
      handleIdleKey(key);
      break;
    case INPUT_MINUTES:
      handleInputKey(key);
      break;
  }
}

// ============================================================
// Key Handlers
// ============================================================
void handleIdleKey(char key) {
  if (key == 'A') {
    // Toggle status and start input
    isInRoom = !isInRoom;
    minutesInput = "";
    currentState = INPUT_MINUTES;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(isInRoom ? "IN ROOM" : "GOING OUT");
    lcd.setCursor(0, 1);
    lcd.print("Minutes: ");
  }
}

void handleInputKey(char key) {
  if (key >= '0' && key <= '9') {
    // Max 3 digits (up to 999 minutes)
    if (minutesInput.length() < 3) {
      minutesInput += key;
      lcd.setCursor(9, 1);
      lcd.print(minutesInput);
      lcd.print("   ");  // Clear trailing chars
    }
  }
  else if (key == '#') {
    // Backspace - delete last digit
    if (minutesInput.length() > 0) {
      minutesInput.remove(minutesInput.length() - 1);
      lcd.setCursor(9, 1);
      lcd.print(minutesInput);
      lcd.print("   ");
    }
  }
  else if (key == '*') {
    // Confirm and send
    if (minutesInput.length() == 0) {
      lcd.setCursor(0, 1);
      lcd.print("Enter a number! ");
      return;
    }
    sendStatus();
  }
  else if (key == 'B') {
    // Cancel - go back to idle
    currentState = IDLE;
    isInRoom = !isInRoom;  // Revert toggle
    showIdleScreen();
  }
}

// ============================================================
// Send Status to Server
// ============================================================
void sendStatus() {
  int minutes = minutesInput.toInt();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sending...");

  if (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Error!");
    lcd.setCursor(0, 1);
    lcd.print("Reconnecting...");

    WiFi.reconnect();
    delay(3000);

    if (WiFi.status() != WL_CONNECTED) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Still no WiFi");
      lcd.setCursor(0, 1);
      lcd.print("B=Cancel");
      return;
    }
  }

  HTTPClient http;
  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  // Build JSON
  JsonDocument doc;
  doc["status"] = isInRoom ? "in_room" : "out";
  doc["minutes"] = minutes;

  String json;
  serializeJson(doc, json);

  Serial.print("POST: ");
  Serial.println(json);

  int httpCode = http.POST(json);

  lcd.clear();
  if (httpCode == 200) {
    lcd.setCursor(0, 0);
    lcd.print(isInRoom ? "IN ROOM" : "OUT");
    lcd.setCursor(0, 1);
    lcd.print("Sent! ");
    lcd.print(minutes);
    lcd.print(" min");
    Serial.println("Success!");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Send Error!");
    lcd.setCursor(0, 1);
    lcd.print("Code: ");
    lcd.print(httpCode);
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
  }

  http.end();

  delay(2000);
  currentState = IDLE;
  showIdleScreen();
}

// ============================================================
// Display Helpers
// ============================================================
void showIdleScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Status: ");
  lcd.print(isInRoom ? "IN ROOM" : "OUT");
  lcd.setCursor(0, 1);
  lcd.print("A=Update B=Cancel");
}
