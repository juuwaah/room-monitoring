#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Keypad.h>
#include <LiquidCrystal.h>
#include <ArduinoJson.h>

// ============================================================
// WiFi Settings
// ============================================================
const char* WIFI_SSID = "b(second)";
const char* WIFI_PASS = "yoyoyoyo";

// ============================================================
// Server Settings
// =======================https://room-monitoring-production.up.railway.app/=====================================
const char* SERVER_URL = "https://room-monitoring-production.up.railway.app/api/status";

// ============================================================
// LCD1602 (4-bit parallel mode)
// RS=GPIO19, E=GPIO23, D4=GPIO18, D5=GPIO17, D6=GPIO16, D7=GPIO15
// ============================================================
LiquidCrystal lcd(19, 23, 18, 17, 16, 15);

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
  STATE_IDLE,
  STATE_INPUT
};

State currentState = STATE_IDLE;
bool isInRoom = true;        // true = in_room, false = out
String minutesInput = "";    // Buffer for digit input

// ============================================================
// Setup
// ============================================================
void setup() {
  Serial.begin(115200);

  // Init LCD (16 cols x 2 rows)
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi");

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
    lcd.print("WiFi Conectado!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
  } else {
    Serial.println("\nWiFi FAILED");
    lcd.setCursor(0, 0);
    lcd.print("WiFi Fallo!");
    lcd.setCursor(0, 1);
    lcd.print("Verificar config");
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
    case STATE_IDLE:
      handleIdleKey(key);
      break;
    case STATE_INPUT:
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
    currentState = STATE_INPUT;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(isInRoom ? "EN SALA" : "SALIENDO");
    lcd.setCursor(0, 1);
    lcd.print("Minutos: ");
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
      lcd.print("Ingrese numero! ");
      return;
    }
    sendStatus();
  }
  else if (key == 'B') {
    // Cancel - go back to idle
    currentState = STATE_IDLE;
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
  lcd.print("Enviando...");

  if (WiFi.status() != WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error WiFi!");
    lcd.setCursor(0, 1);
    lcd.print("Reconectando...");

    WiFi.reconnect();
    delay(3000);

    if (WiFi.status() != WL_CONNECTED) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sin WiFi");
      lcd.setCursor(0, 1);
      lcd.print("B=Cancelar");
      return;
    }
  }

  WiFiClientSecure client;
  client.setInsecure();  // Skip SSL certificate verification

  HTTPClient http;
  http.begin(client, SERVER_URL);
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
    lcd.print(isInRoom ? "EN SALA" : "AUSENTE");
    lcd.setCursor(0, 1);
    lcd.print("Enviado! ");
    lcd.print(minutes);
    lcd.print(" min");
    Serial.println("Success!");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Error envio!");
    lcd.setCursor(0, 1);
    lcd.print("Codigo: ");
    lcd.print(httpCode);
    Serial.print("HTTP Error: ");
    Serial.println(httpCode);
  }

  http.end();

  delay(2000);
  currentState = STATE_IDLE;
  showIdleScreen();
}

// ============================================================
// Display Helpers
// ============================================================
void showIdleScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Estado: ");
  lcd.print(isInRoom ? "EN SALA" : "AUSENTE");
  lcd.setCursor(0, 1);
  lcd.print("A=Cambiar B=Canc");
}
