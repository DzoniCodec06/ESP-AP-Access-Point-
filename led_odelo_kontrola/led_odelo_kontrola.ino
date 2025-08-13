#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h> // or #include <SPIFFS.h>
#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(81);

WebServer server(80);

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("[%u] Connected\n", num);
      webSocket.sendTXT(num, "Hello from ESP32!");
      break;

    case WStype_TEXT:
      Serial.printf("[%u] Received: %s\n", num, payload);
      webSocket.sendTXT(num, "ESP32 got: " + String((char*)payload));
      break;
  }
}

void setup() {
  Serial.begin(115200);

  // Start Access Point
  WiFi.softAP("LED-ODELO", "");
  Serial.println("Access Point started");
  Serial.println(WiFi.softAPIP());

  // Initialize filesystem
  if (!LittleFS.begin()) {
    Serial.println("Filesystem mount failed");
    return;
  }

  // Serve static files
  server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  server.on("/style.css", HTTP_GET, []() {
    File file = LittleFS.open("/style.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  });

  server.on("/app.js", HTTP_GET, []() {
    File file = LittleFS.open("/app.js", "r");
    server.streamFile(file, "application/javascript");
    file.close();
  });

  server.on("/animation-icon.png", HTTP_GET, []() {
    File file = LittleFS.open("/animation-icon.png", "r");
    server.streamFile(file, "image/png");
    file.close();
  });

  server.on("/array-icon.png", HTTP_GET, []() {
    File file = LittleFS.open("/array-icon.png", "r");
    server.streamFile(file, "image/png");
    file.close();
  });

  server.on("/fast-icon.png", HTTP_GET, []() {
    File file = LittleFS.open("/fast-icon.png", "r");
    server.streamFile(file, "image/png");
    file.close();
  });

  server.on("/slow-icon.png", HTTP_GET, []() {
    File file = LittleFS.open("/slow-icon.png", "r");
    server.streamFile(file, "image/png");
    file.close();
  });

  server.on("/sun-icon.png", HTTP_GET, []() {
    File file = LittleFS.open("/sun-icon.png", "r");
    server.streamFile(file, "image/png");
    file.close();
  });

  server.on("/sun-little.png", HTTP_GET, []() {
    File file = LittleFS.open("/sun-little.png", "r");
    server.streamFile(file, "image/png");
    file.close();
  });

  server.on("/palette-icon.png", HTTP_GET, []() {
    File file = LittleFS.open("/palette-icon.png", "r");
    server.streamFile(file, "image/png");
    file.close();
  });

  server.begin();
  Serial.println("Web server started");

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
}