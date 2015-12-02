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
  String str;
  int index;
  // Execute.
  currentMode = CUSTOM_SHOW;
  // Execute.
  for ( uint8_t i = 0; i < server.args(); i++ ) {
    index = server.argName(i).toInt();
    if (index == 0) {
      continue;
    }
    index--;
    str = server.arg(i);
    char r[5] = {0};
    char g[5] = {0};
    char b[5] = {0};
    r[0] = g[0] = b[0] = '0';
    r[1] = g[1] = b[1] = 'X';
    r[2] = str[3];
    r[3] = str[4];
    g[2] = str[5];
    g[3] = str[6];
    b[2] = str[7];
    b[3] = str[8];
    setColor(index, strtol(r, NULL, 16), strtol(g, NULL, 16), strtol(b, NULL, 16));
  }
  server.send(200, "application/javascript", "OK");
}
