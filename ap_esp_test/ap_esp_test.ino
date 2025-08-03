#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

ESP8266WebServer server(80);


void handleFormSubmit() {
  String poruka = server.arg("poruka");
  String color = server.arg("boja");

  Serial.println("Received values:");
  Serial.println("Poruka: " + poruka);
  Serial.println("Boja: " + color);

  // Redirect back to "/"
  server.sendHeader("Location", "/", true); // true = 302 redirect
  server.send(302, "text/plain", "");
  
}

void setup() {
  Serial.begin(115200);
  WiFi.softAP("MY-ESP", "");
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

  server.on("/submit", HTTP_GET, handleFormSubmit);  // For GET method

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}
