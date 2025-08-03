
#include <Ticker.h>
#include <PxMatrix.h>  //https://github.com/2dom/PxMatrix

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

ESP8266WebServer server(80);

Ticker display_ticker;

//Pin Definition for Nodemcu to HUB75 LED MODULE
#define P_LAT 16  //nodemcu pin D0
#define P_A 5     //nodemcu pin D1
#define P_B 4     //nodemcu pin D2
#define P_C 15    //nodemcu pin D8
#define P_OE 2    //nodemcu pin D4
#define P_D 12    //nodemcu pin D6
#define P_E 0     //nodemcu pin GND // no connection
//no clk pin no e pin in hub 75

// PxMATRIX display(32,16,P_LAT, P_OE,P_A,P_B,P_C);
PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);
//PxMATRIX display(64, 64, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);

// Some standard colors
uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myBLACK = display.color565(0, 0, 0);

uint16 myCOLORS[8] = { myRED, myGREEN, myBLUE, myWHITE, myYELLOW, myCYAN, myMAGENTA, myBLACK };

// ISR for display refresh
void display_updater() {
  display.display(70);
}

String poruka;
String prevPoruka;
String color;
String form_repeat;

bool home;

bool repeating = false;

bool one_time = false;

unsigned long run_time;
unsigned long delay_time = 1000;
unsigned long prev_time = 0;

void handleFormSubmit() {
  home = false;
  poruka = server.arg("poruka");
  color = server.arg("boja");
  form_repeat = server.arg("ponavljanje");

  if (form_repeat != "") {
    repeating = true;
  } else if (form_repeat == "") {
    repeating = false;
  }

  Serial.println("Received values:");
  Serial.println("Poruka: " + poruka);
  Serial.println("Boja: " + color);
  Serial.println("Ponavljanje: " + repeating);

  // Redirect back to "/"
  delay(1000);
  server.sendHeader("Location", "/", true);  // true = 302 redirect
  delay(1000);
  server.send(302, "text/plain", "");

  one_time = false;

  //This will display the scrolling text.
}

void setup() {

  // Define your display layout here, e.g. 1/8 step
  display.begin(16);
  display.clearDisplay();
  display.setTextColor(myCYAN);
  Serial.begin(115200);

  WiFi.softAP("LED-DISPLAY", "");
  Serial.println("Access Point started");
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());  // Usually 192.168.4.1

  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS access faild");
    return;
  }

  server.on("/", HTTP_GET, []() {
    File file = SPIFFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();

    Serial.println("Home /");

    home = true;
  });

  server.on("/style.css", HTTP_GET, []() {
    File file = SPIFFS.open("/style.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/app.js", HTTP_GET, []() {
    File file = SPIFFS.open("/app.js", "r");
    server.streamFile(file, "application/javascript");
    file.close();
  });

  server.on("/bckg2.png", HTTP_GET, []() {
    File file = SPIFFS.open("/bckg2.png", "r");
    server.streamFile(file, "image/png");  // or "image/png" if using PNG
    file.close();
  });

  server.on("/submit", HTTP_GET, handleFormSubmit);  // For GET method

  server.begin();
  Serial.println("Web server started");

  display_ticker.attach(0.002, display_updater);
  yield();
  delay(1000);
  display.clearDisplay();
}

unsigned long last_draw = 0;

int generate_random_number() {
  int rn = random(255);
  return rn;
}

bool hexToRGB(String hex, uint8_t &r, uint8_t &g, uint8_t &b) {
  if (hex.startsWith("#")) {
    hex = hex.substring(1);
  }
  // Validate length
  if (hex.length() != 6) return false;

  // Convert hex substrings to integers
  r = strtoul(hex.substring(0, 2).c_str(), NULL, 16);
  g = strtoul(hex.substring(2, 4).c_str(), NULL, 16);
  b = strtoul(hex.substring(4, 6).c_str(), NULL, 16);

  return true;
}

void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB, bool rainbow, bool repeat) {
  uint16_t text_length = text.length();
  display.setTextWrap(false);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  //display.setTextColor(display.color565(colorR, colorG, colorB));
  uint8_t r, g, b;

  if (hexToRGB(color, r, g, b)) {
    display.setTextColor(display.color565(r, g, b));
  } else display.setTextColor(display.color565(255, 0, 0));

  int n, j, l;
  if (repeat) {
    for (int xpos = 64; xpos > -(32 + text_length * 5); xpos--) {
      if (rainbow) {
        //n < 255 ? n += 1 : n = 255;
        //l < 255 ? l += 1 : l = 255;
        //j > 0 ? j -= 1 : j = 0;
        n = generate_random_number();
        delay(1);
        j = generate_random_number();
        delay(1);
        l = generate_random_number();
        delay(1);

        display.setTextColor(display.color565(n, j, l));
      } else {
        display.setTextColor(display.color565(r, g, b));
      }
      display.clearDisplay();
      display.setCursor(xpos, ypos);
      display.println(text);

      delay(scroll_delay / 8);
      yield();
      //Serial.println("scroll func");
    }
  } else if (repeat == false && one_time == false) {
    for (int xpos = 64; xpos > -(32 + text_length * 5); xpos--) {
      if (rainbow) {
        //n < 255 ? n += 1 : n = 255;
        //l < 255 ? l += 1 : l = 255;
        //j > 0 ? j -= 1 : j = 0;
        n = generate_random_number();
        delay(1);
        j = generate_random_number();
        delay(1);
        l = generate_random_number();
        delay(1);

        display.setTextColor(display.color565(n, j, l));
      } else {
        display.setTextColor(display.color565(r, g, b));
      }
      display.clearDisplay();
      display.setCursor(xpos, ypos);
      display.println(text);

      delay(scroll_delay / 8);
      yield();
      //Serial.println("scroll func");
    }

    one_time = true;
  }
  //  5 pixel character width
}

void loop() {
  run_time = millis();

  server.handleClient();
  //This will display the scrolling text.
  //Serial.println("loop func");
  if (prevPoruka != "" && prevPoruka != poruka) {
    delay(1000);
    server.sendHeader("Location", "/", true);  // true = 302 redirect
    delay(1000);
    server.send(302, "text/plain", "");
  }
  if (poruka != "" && home) {
    scroll_text(12, 200, String(poruka), 0, 0, 255, false, repeating);
    delay(20);
    prevPoruka = poruka;
  }
}
