#include <Ticker.h>
#include <PxMatrix.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <math.h> // for sin/cos

#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.017453292519943295769 // PI/180
#endif

ESP8266WebServer server(80);
Ticker display_ticker;

// ===== Display Pins =====
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_D 12
#define P_OE 2

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

// ===== Display Init =====
PxMATRIX display(DISPLAY_WIDTH, DISPLAY_HEIGHT, P_LAT, P_OE, P_A, P_B, P_C, P_D);

// ===== Timing =====
unsigned long lastFrameTime = 0;
const unsigned long frameInterval = 50; // ms

// ===== Fireworks State =====
struct Firework {
  int centerX, centerY;
  int radius;
  uint16_t color;
  bool active;
};

Firework fw = {0, 0, 0, 0, false};

// ===== Heartbeat =====
int hbSize = 1;
int hbDirection = 1;
const uint8_t heartBitmap[] PROGMEM = {
  0b01100110,
  0b11111111,
  0b11111111,
  0b11111111,
  0b01111110,
  0b00111100,
  0b00011000,
  0b00000000
};

void updateFireworks() {
  if (!fw.active) {
    fw.centerX = random(DISPLAY_WIDTH);
    fw.centerY = random(DISPLAY_HEIGHT);
    fw.radius = 0;
    fw.color = display.color565(random(255), random(255), random(255));
    fw.active = true;
  }

  for (int i = 0; i < 360; i += 20) {
    float rad = i * DEG_TO_RAD;
    int x = fw.centerX + cos(rad) * fw.radius;
    int y = fw.centerY + sin(rad) * fw.radius;
    if (x >= 0 && x < DISPLAY_WIDTH && y >= 0 && y < DISPLAY_HEIGHT) {
      display.drawPixel(x, y, fw.color);
    }
  }

  fw.radius++;
  if (fw.radius > 8) fw.active = false;
}

void updateHeartbeat() {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (pgm_read_byte(&heartBitmap[y]) & (0x80 >> x)) {
        for (int dx = 0; dx < hbSize; dx++) {
          for (int dy = 0; dy < hbSize; dy++) {
            display.drawPixel(x * hbSize + dx + 20, y * hbSize + dy + 8, display.color565(255, 0, 0));
          }
        }
      }
    }
  }
  hbSize += hbDirection;
  if (hbSize > 3 || hbSize < 1) hbDirection *= -1;
}

// ===== Display refresh ISR =====
void display_updater() {
  display.display(70);
}

// ===== Color parser =====
bool hexToRGB(String hex, uint8_t &r, uint8_t &g, uint8_t &b) {
  if (hex.startsWith("#")) hex = hex.substring(1);
  if (hex.length() != 6) return false;
  r = strtoul(hex.substring(0, 2).c_str(), NULL, 16);
  g = strtoul(hex.substring(2, 4).c_str(), NULL, 16);
  b = strtoul(hex.substring(4, 6).c_str(), NULL, 16);
  return true;
}

// ===== Scroll State =====
String scrollMessage = "";
int16_t scrollX = 0;
uint8_t scrollY = 12;
unsigned long lastScrollMove = 0;
unsigned long scrollSpeed = 50;
bool scrollRepeat = false;
uint16_t scrollColor;

String mode, scroll, poruka, color;

String lastPoruka = "";
String lastColor = "";
String lastMode = "";

void handleFormSubmit() {
  poruka = server.arg("poruka");
  color = server.arg("boja");
  scroll = server.arg("scroll");
  mode = server.arg("mode");
  scrollRepeat = (server.arg("ponavljanje") != "");

  uint8_t r, g, b;
  if (!hexToRGB(color, r, g, b)) { r = 255; g = 0; b = 0; }
  scrollColor = display.color565(r, g, b);

  scrollMessage = poruka;
  scrollX = display.width();

  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleFileRequest() {
  String path = server.uri();
  if (path.endsWith("/")) path += "index.html";
  if (!SPIFFS.exists(path)) {
    server.send(404, "text/plain", "404: File Not Found");
    return;
  }

  String contentType = "text/plain";
  if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".css")) contentType = "text/css";
  else if (path.endsWith(".js")) contentType = "application/javascript";
  else if (path.endsWith(".png")) contentType = "image/png";
  else if (path.endsWith(".jpg") || path.endsWith(".jpeg")) contentType = "image/jpeg";
  else if (path.endsWith(".ico")) contentType = "image/x-icon";

  File file = SPIFFS.open(path, "r");
  server.sendHeader("Cache-Control", "max-age=86400");
  server.streamFile(file, contentType);
  file.close();
}

void updateScroll() {
  if (scroll != "on" || scrollMessage == "") return;

  unsigned long now = millis();
  if (now - lastScrollMove >= scrollSpeed) {
    lastScrollMove = now;
    display.clearDisplay();
    display.setCursor(scrollX, scrollY);
    display.setTextColor(scrollColor);
    display.print(scrollMessage);

    scrollX--;
    int textWidth = scrollMessage.length() * 6;
    if (scrollX < -textWidth) {
      if (scrollRepeat) scrollX = display.width();
      else scrollMessage = "";
    }
  }
}

void showCenteredText(String txt, String clr) {
  uint8_t r, g, b;
  if (!hexToRGB(clr, r, g, b)) { r = 255; g = 0; b = 0; }
  uint16_t textColor = display.color565(r, g, b);

  display.clearDisplay();
  int textWidth = txt.length() * 6;
  int textHeight = 8;
  int x = (display.width() - textWidth) / 2;
  int y = (display.height() - textHeight) / 2;
  display.setCursor(x, y);
  display.setTextColor(textColor);
  display.print(txt);
}

void setup() {
  Serial.begin(115200);

  display.begin(16); // Most 64x32 panels are 1/16 scan
  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextSize(1);

  WiFi.softAP("LED-DISPLAY", "");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS init failed");
    return;
  }

  server.onNotFound(handleFileRequest);
  server.on("/submit", HTTP_GET, handleFormSubmit);
  server.begin();
  Serial.println("Web server started");
  display_ticker.attach(0.004, display_updater); // 250Hz
}

void loop() {
  server.handleClient();
  if (scroll == "on") {
    updateScroll();
  }
  else if (mode == "text") {
    if (poruka != lastPoruka || color != lastColor || mode != lastMode) {
      showCenteredText(poruka, color);
      lastPoruka = poruka;
      lastColor = color;
      lastMode = mode;
    }
  }

  if (mode == "solid") {
    if (millis() - lastFrameTime >= frameInterval) {
      lastFrameTime = millis();
      display.clearDisplay();
      static bool doFireworks = true;
      if (doFireworks) {
        updateFireworks();
        if (!fw.active) doFireworks = false;
      } else {
        updateHeartbeat();
        static int hbFrames = 0;
        hbFrames++;
        if (hbFrames > 20) { hbFrames = 0; doFireworks = true; }
      }
    }
  }
}
