#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

static bool str_to_uint16(const char *str, uint16_t *res) {
  long int val = strtol(str, NULL, 10);
  *res = (uint16_t) val;
  return true;
}

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
  DynamicJsonBuffer jsonBuffer;
  // Parse incoming JSON.
  JsonObject& incoming = jsonBuffer.parseObject(json);

  Serial.println(String(ESP.getFreeHeap()));

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
    if (incoming["values"].is<JsonArray&>()) {
      Serial.println("Values is array.");
      JsonArray& values = incoming["values"].as<JsonArray&>();
      uint16_t i = 0;
      for (JsonArray::iterator it=values.begin(); (it!=values.end()) && (i < strip->numPixels()); ++it) {
        // uint32_t color = hexToInt(values[i].asString());
        // Serial.println(color);
        animation1.setDefaultFrameByIndex(i, animation1.toInt(it->asString()));
        i++;
      }
    }
    if (incoming["values"].is<JsonObject&>()) {
      Serial.println("Values is object.");
      JsonObject& values = incoming["values"].as<JsonObject&>();
      for (JsonObject::iterator it=values.begin(); it!=values.end(); ++it) {
        // uint32_t color = hexToInt(values[i].asString());
        // Serial.println(color);
        uint16_t key;

        str_to_uint16(it->key, &key);

        Serial.printf("Key %s %d\n", it->key, key);
        // int value = it->value;
        animation1.setDefaultFrameByIndex(key, animation1.toInt(it->value.asString()));
      }
    }
  }
  return true;
}

String getCurrentState() {
  StaticJsonBuffer<2500> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root.set("program", (int)animation1.getProgram());
  root.set("brightness", animation1.getStrip()->getBrightness());

  JsonArray& values = root.createNestedArray("values");

  Adafruit_NeoPixel* strip = animation1.getStrip();

  // Read values.
  for (uint16_t i=0; i < strip->numPixels(); i++) {
    // Add values to array.
    uint32_t color = animation1.getDefaultFrameByIndex(i);
    Serial.println(color);
    String buf = animation1.toHex(color);
    Serial.println(i);
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
  // webSocket.broadcastTXT(getCurrentState());
  server.send(200, "application/json", getCurrentState());
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;

    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        String info = getCurrentState();
        webSocket.sendTXT(num, info);
      }
      break;

    case WStype_TEXT:
      {
        Serial.printf("[%u] get Text: %s\n", num, payload);
        if(payload[0] == '#') {
          uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);
          animation1.setColor(rgb);
        }
        if (payload[0] == '{') {
          parseData((const char *) &payload[0]);
        }
        // send data to all connected clients
        String info = getCurrentState();
        webSocket.broadcastTXT(info);
      }
      break;

    case WStype_BIN:
      Serial.printf("[%u] get binary lenght: %u\n", num, lenght);
      hexdump(payload, lenght);

      // send message to client
      // webSocket.sendBIN(num, payload, lenght);
      break;
  }
}
