/*
  To upload through terminal you can use: curl -F "image=@christmas-tree.cpp.generic.bin" gitree.local/update
*/

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

void handleRoot() {
 // Define.
  StaticJsonBuffer<1600> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  char tmp[6];

  root["currentMode"] = currentMode;
  root["currentColor"] = currentColor;
  root["currentLed"] = currentLed;
  JsonArray& colors = root.createNestedArray("colors");

  // Read values.
  for ( uint8_t i = 0; i < strip.numPixels(); i++ ) {
    colors.add(intToHex(strip.getPixelColor(i)));
  }

  // Print.
  String temp;
  root.printTo(temp);
  server.send(200, "application/javascript", temp);
}

void handleSetMode() {
  // Define.
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  // Execute.
  for ( uint8_t i = 0; i < server.args(); i++ ) {
    if (server.argName(i) == "mode") {
      currentMode = server.arg(i).toInt();
    }
    if (server.argName(i) == "color") {
      currentColor = server.arg(i);
    }
    if (server.argName(i) == "led") {
      currentLed = server.arg(i).toInt() - 1;
    }
  }
  startShow(currentMode);
  root["currentMode"] = currentMode;
  root["currentColor"] = currentColor;
  root["currentLed"] = currentLed;

  // Print.
  String temp;
  root.printTo(temp);
  server.send(200, "application/javascript", temp);
}

void handleSetRandomMode() {
  // Define.
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  // Execute.
  // Get random mode
  currentMode = random(-1, 6);
  root["currentMode"] = currentMode;
  // Get random color.
  currentColor = intToHex(random(0, 2000));
  root["currentColor"] = currentColor;
  // Get random led.
  currentLed = random(0, strip.numPixels() - 1);
  root["currentLed"] = currentLed;

  // Print.
  String temp;
  root.printTo(temp);
  server.send(200, "application/javascript", temp);
}

void handleSetValue() {
  // Define.
  StaticJsonBuffer<2000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  char tmp[6];

  // Change mode.
  currentMode = CUSTOM_SHOW;
  root["currentMode"] = currentMode;

  // Execute.
  for ( uint8_t i = 0; i < server.args(); i++ ) {
    if (server.argName(i).toInt() == 0) {
      continue;
    }
    strip.setPixelColor(server.argName(i).toInt() - 1, hexToInt(server.arg(i)));
  }
  strip.show();

  JsonArray& colors = root.createNestedArray("colors");

  // Read values.
  for ( uint8_t i = 0; i < strip.numPixels(); i++ ) {
    colors.add(intToHex(strip.getPixelColor(i)));
  }

  // Print.
  String temp;
  root.printTo(temp);
  server.send(200, "application/javascript", temp);
}

