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

void handleStatus() {
  StaticJsonBuffer<2000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["hostname"] = HOSTNAME;
  root["repository"] = REPOSITORY;

  JsonObject& settings = root.createNestedObject("settings");

  settings["PIXEL_COUNT"] = PIXEL_COUNT;
  settings["PIXEL_OFFSET"] = PIXEL_OFFSET;
  settings["DEFAULT_SHOW"] = DEFAULT_SHOW;
  settings["DEFAULT_COLOR"] = DEFAULT_COLOR;
  settings["DEFAULT_LED"] = DEFAULT_LED;

  String temp;
  root.printTo(temp);
  server.send(200, "application/json", temp);
}

void parseData(String data) {
  // Define.
  StaticJsonBuffer<2000> jsonBuffer;
  // Parse incoming JSON.
  JsonObject& parsed = jsonBuffer.parseObject(data);

  if (parsed.containsKey("global")) {
    JsonObject& global = parsed["global"];

    if (global.containsKey("mode")) {
      currentMode = global["mode"].as<int>();
      Serial.println(currentMode);
    }

   if (global.containsKey("color")) {
      Serial.println(currentColor);
      Serial.println(global["color"].asString());
      currentColor = global["color"].asString();
      Serial.println(currentColor);
    }

    if (global.containsKey("led")) {
      currentLed = global["led"].as<int>();;
      Serial.println(currentLed);
    }
  }

  if (parsed.containsKey("values")) {
    JsonArray& values = parsed["values"].asArray();
    for (int i = 0; i < sizeof(values); i++) {
      Serial.println(i);
      strip.setPixelColor(i, hexToInt(values[i]));
    }
  }
}

JsonObject& getCurrentState() {
  StaticJsonBuffer<2000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  JsonArray& values = root.createNestedArray("values");
  // Read values.
  for ( uint8_t i = 0; i < strip.numPixels(); i++ ) {
    // Add values to array.
    values.add(intToHex(strip.getPixelColor(i)));
  }

  JsonObject& global = root.createNestedObject("global");
  global.set("mode", currentMode);
  global.set("color", currentColor);
  global.set("led", currentLed);

  return root;
}

void handleGet() {
  JsonObject& root = getCurrentState();

  // Print.
  String temp;
  root.printTo(temp);
  server.send(200, "application/json", temp);
}

void handlePost() {

  // Parse request.
  parseData(server.arg(0));

  // Prepare response.
  JsonObject& root = getCurrentState();
  String temp;
  root.printTo(temp);
  server.send(200, "application/json", temp);
}

