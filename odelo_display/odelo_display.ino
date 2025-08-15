#include <Ticker.h>
#include <PxMatrix.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

ESP8266WebServer server(80);
Ticker display_ticker;

// ===== Display Pins =====
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_OE 2
#define P_D 12
#define P_E 0

// ===== Display Init (64x32, 1/16 scan) =====
PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);

// ===== Refresh ISR =====
void display_updater() {
  display.display(70); // Refresh the display
}

// ===== Scroll State =====
String scrollMessage = "";
int16_t scrollX = 0;
uint8_t scrollY = 12;
unsigned long lastScrollMove = 0;
unsigned long scrollSpeed = 50; // ms per pixel
bool scrollRepeat = false;
uint16_t scrollColor = display.color565(255, 0, 0);

// ===== Form Handling =====
bool hexToRGB(String hex, uint8_t &r, uint8_t &g, uint8_t &b) {
  if (hex.startsWith("#")) hex = hex.substring(1);
  if (hex.length() != 6) return false;
  r = strtoul(hex.substring(0, 2).c_str(), NULL, 16);
  g = strtoul(hex.substring(2, 4).c_str(), NULL, 16);
  b = strtoul(hex.substring(4, 6).c_str(), NULL, 16);
  return true;
}

String mode;

void handleFormSubmit() {
  String poruka = server.arg("poruka");
  String color = server.arg("boja");
  String form_repeat = server.arg("ponavljanje");
  mode = server.arg("mode");

  Serial.println(mode);

  scrollRepeat = (form_repeat != "");

  uint8_t r, g, b;
  if (!hexToRGB(color, r, g, b)) {
    r = 255; g = 0; b = 0;
  }
  scrollColor = display.color565(r, g, b);

  scrollMessage = poruka;
  scrollX = display.width(); // Start just off right edge

  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

// ===== Serve Files from SPIFFS =====
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

// ===== Setup =====
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

  display_ticker.attach(0.002, display_updater);
}

// ===== Scroll Update (non-blocking) =====
void updateScroll() {
  if (scrollMessage == "") return;

  unsigned long now = millis();
  if (now - lastScrollMove >= scrollSpeed) {
    lastScrollMove = now;

    display.clearDisplay();
    display.setCursor(scrollX, scrollY);
    display.setTextColor(scrollColor);
    display.print(scrollMessage);

    scrollX--;
    int textWidth = scrollMessage.length() * 6; // 6 px per char
    if (scrollX < -textWidth) {
      if (scrollRepeat) {
        scrollX = display.width();
      } else {
        scrollMessage = ""; // Stop
      }
    }
  }
}

bool first_solid = false;
unsigned long prevTime = 0;

void animations() {
  if (mode == "solid") {
    unsigned long now = millis();

    first_solid = false;

    if (now - prevTime >= 1000) {
      display.fillRect(0, 0, 64, 32, display.color565(random(255), random(255), random(255))); // Fill top half of panel in red
      prevTime = now;
    }
  } else {
    if (!first_solid) {
      first_solid = true;
      display.clearDisplay();
      return;
    } else return;
  }
}

// ===== Loop =====
void loop() {
  server.handleClient();
  updateScroll();
  animations();
}
