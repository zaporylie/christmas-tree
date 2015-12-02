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
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  snprintf(temp, 400,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #00ff00; font-family: Arial, Helvetica, Sans-Serif; Color: #ffffff; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266-gitree!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
  </body>\
</html>",

    hr, min % 60, sec % 60
  );
  server.send ( 200, "text/html", temp );
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
  }
  startShow(currentMode);
  root["currentMode"] = currentMode;

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
  currentMode = random(0, 9);
  root["currentMode"] = currentMode;

  // Print.
  String temp;
  root.printTo(temp);
  server.send(200, "application/javascript", temp);
}

void handleSetValue() {
  // Define.
  StaticJsonBuffer<1600> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  String str;
  int index;

  // Change mode.
  currentMode = CUSTOM_SHOW;
  root["currentMode"] = currentMode;

  // Execute.
  for ( uint8_t i = 0; i < server.args(); i++ ) {
    index = server.argName(i).toInt();
    if (index == 0) {
      continue;
    }
    index--;
    str = server.arg(i);
    strip.setPixelColor(index, hexToInt(str));
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
