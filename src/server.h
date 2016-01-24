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
  settings["DEFAULT_COLOR"] = DEFAULT_COLOR;

  String temp;
  root.printTo(temp);
  server.send(200, "application/json", temp);
}

bool parseData(String json) {
  // Define.
  StaticJsonBuffer<3000> jsonBuffer;
  // Parse incoming JSON.
  JsonObject& incoming = jsonBuffer.parseObject(json);

  if (!incoming.success()) {
    return false;
  }

  // Strip.
  Adafruit_NeoPixel* strip = animation1.getStrip();

  if (incoming.containsKey("program")) {
    // currentMode = incoming["mode"].as<int>();
    animation1.setProgram(static_cast<AnimationProgram>(incoming["program"].as<int>()));
  }

  if (incoming.containsKey("color")) {
    animation1.setColor(animation1.toInt(incoming["color"].asString()));
  }

  if (incoming.containsKey("brightness")) {
    strip->setBrightness(incoming["brightness"].as<uint8_t>());
  }

  if (incoming.containsKey("values")) {

    Serial.println("values");
    JsonArray& values = incoming["values"].as<JsonArray&>();
    uint16_t i = 0;
    for (JsonArray::iterator it=values.begin(); (it!=values.end()) && (i < strip->numPixels()); ++it) {
      // uint32_t color = hexToInt(values[i].asString());
      // Serial.println(color);
      animation1.setDefaultFrameByIndex(i, animation1.toInt(it->asString()));
      i++;
    }
  }
  return true;
}

String getCurrentState() {
  StaticJsonBuffer<3000> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root.set("program", (int)animation1.getProgram());
  root.set("brightness", animation1.getStrip()->getBrightness());

  JsonArray& values = root.createNestedArray("values");

  Adafruit_NeoPixel* strip = animation1.getStrip();

  // Read values.
  for (uint16_t i = 0; i < strip->numPixels(); i++) {
    // Add values to array.
    char buf[7];
    animation1.toHex(buf, strip->getPixelColor(i));
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
  if (!parseData(server.arg(0))) {
    server.send(400, "application/json", "Invalid json");
  }
  server.send(200, "application/json", getCurrentState());
}
