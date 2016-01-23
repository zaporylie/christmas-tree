/*
  To upload through terminal you can use: curl -F "image=@christmas-tree.cpp.generic.bin" gitree.local/update
*/

ESP8266WebServer server(80);

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
  const int BUFFER_SIZE = JSON_OBJECT_SIZE(11);
  Serial.println(BUFFER_SIZE);
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["hostname"] = HOSTNAME;
  root["repository"] = REPOSITORY;

  JsonObject& settings = root.createNestedObject("settings");

  settings["PIXEL_COUNT"] = PIXEL_COUNT;
  settings["PIXEL_OFFSET"] = PIXEL_OFFSET;
  settings["DEFAULT_SHOW"] = DEFAULT_SHOW;
  settings["DEFAULT_COLOR"] = DEFAULT_COLOR;
  settings["DEFAULT_LED"] = DEFAULT_LED;
  settings["FPS"] = FPS;
  settings["FPS_TIME"] = FPS_TIME;

  String temp;
  root.printTo(temp);
  server.send(200, "application/json", temp);
}

void parseData(String json) {
  // Define.
  StaticJsonBuffer<3000> jsonBuffer;
  // Parse incoming JSON.
  JsonObject& parsed = jsonBuffer.parseObject(json);

  if (parsed.containsKey("mode")) {
    currentMode = parsed["mode"].as<int>();
    Serial.println(currentMode);
  }

 if (parsed.containsKey("color")) {
    fillNextFrame(hexToInt(parsed["color"].asString()));
  }

  if (parsed.containsKey("values")) {
    Serial.println("values");
    JsonArray& values = parsed["values"].as<JsonArray&>();
    uint16_t i = 0;
    for (JsonArray::iterator it=values.begin(); it!=values.end(); ++it) {
      // uint32_t color = hexToInt(values[i].asString());
      // Serial.println(color);
      nextFrame[i] = hexToInt(it->asString());
      i++;
    }
  }
}

String getCurrentState() {
  StaticJsonBuffer<3000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root.set("mode", currentMode);

  JsonArray& values = root.createNestedArray("values");

  // Read values.
  for ( uint16_t i = 0; i < strip.numPixels(); i++ ) {
    // Serial.println(i);
    // Add values to array.
    String buf = intToHex(strip.getPixelColor(i));
    values.add(buf);
  }

  String temp;
  root.printTo(temp);
  return temp;
}

void handleGet() {
  server.send(200, "application/json", getCurrentState());
}

void handlePost() {
  // Parse request.
  parseData(server.arg(0));
  server.send(200, "application/json", getCurrentState());
}
