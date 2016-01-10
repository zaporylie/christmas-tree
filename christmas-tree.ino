#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h> 
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include "settings.h"
#include "pixel.h"
#include "server.h"


void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show();
  startShow(DEFAULT_SHOW);

  WiFiManager wifi;
  wifi.autoConnect(HOSTNAME);

  if ( MDNS.begin(HOSTNAME) ) {
    Serial.println("MDNS responder started");
  }

  server.on ( "/", handleRoot );
  server.on ( "/autoDiscover", handleAutoDiscover );
  server.on ( "/setRandomMode", handleSetRandomMode );
  server.on ( "/setMode", handleSetMode );
  server.on ( "/setValue", handleSetValue );
  server.onNotFound ( handleNotFound );
  httpUpdater.setup(&server);
  server.begin();
  Serial.println ( "HTTP server started" );
}

void loop() {
  server.handleClient();
  startShow(currentMode);
}
