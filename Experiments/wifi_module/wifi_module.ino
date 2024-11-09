// requires the following in Arduino/Libraries:
// https://github.com/Links2004/arduinoWebSockets
// https://github.com/bblanchon/ArduinoJson

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ArduinoJson.h>
#include <SerialPacketizer.h>
#include <Arduino.h>
#ifdef OTA
#include <ArduinoOTA.h>
#endif
#include "FS.h"

//////////////////////
// WiFi Definitions //
//////////////////////
const char default_ssid[] = "neos_test";
const char default_pass[] = "MdABffm8cn7a6GwP";
const char default_name[] = "neos";
const char* wifi_ssid;
const char* wifi_pass;
const char* name;
uint baud = 9600;

//WiFiServer server(80);
WebSocketsServer webSocket(8000);

// serial interface
#define HW_SERIAL Serial
SerialPacketizer *sp;

#define CONFIG_BUFFER_LEN 512
char config_buf[CONFIG_BUFFER_LEN];

void wifi_log(const char *msg, int conn, uint32_t ip) {
  StaticJsonBuffer<256> jsonBuffer;
  char outputBuffer[256];
  JsonObject& root = jsonBuffer.createObject();
  root["type"] = "log";
  root["msg"] = msg;
  if (conn != -1) {
    root["conn"] = conn;
  }
  if (ip != 0) {
    IPAddress ipaddr(ip);
    JsonArray& iparray = root.createNestedArray("ip");
    iparray.add(ipaddr[0]);
    iparray.add(ipaddr[1]);
    iparray.add(ipaddr[2]); 
    iparray.add(ipaddr[3]);
  }
  root["time"] = millis();
  root.printTo(outputBuffer, sizeof(outputBuffer));
  sp->send(255, (uint8_t*)outputBuffer, strlen(outputBuffer));
}

void wifi_log(const char *msg, int conn) {
  wifi_log(msg, conn, 0);
}

void wifi_log(const char *msg) {
  wifi_log(msg, -1);
}

void wifi_log(String msg) {
  wifi_log(msg.c_str());
}

void setup() 
{
  HW_SERIAL.begin(baud);
  sp = new SerialPacketizer(&HW_SERIAL);
  // data from serial
  sp->onEvent(
    [&] (uint8_t mode, uint8_t *payload, size_t length) {
      if (mode != 0) {
        return;
      }
      // send data to all connected clients
      webSocket.broadcastTXT(payload, length);
    }
  );
  loadConfig();
  HW_SERIAL.end();
  HW_SERIAL.begin(baud); // use updated baud from config
  setupWifi();
  WiFi.printDiag(HW_SERIAL);
}

void loop() 
{
  yield(); // yield to background network processing
  webSocket.loop();
  yield();
  sp->process();
  yield();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
    wifi_log("Disconnected", num);
    break;
    
    case WStype_CONNECTED:
    {
      IPAddress ip = webSocket.remoteIP(num);
      wifi_log("Connected", num, ip);
    }
    break;
    
    case WStype_TEXT:
    //USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
    sp->send(0, payload, length);
    break;
    
    case WStype_BIN:
    wifi_log("Received binary data", num);
    //USE_SERIAL.printf("[%u] get binary lenght: %u\n", num, length);
    //hexdump(payload, length);

    // echo data back to browser
    //webSocket.sendBIN(num, payload, length);
    break;
  }
}

void loadConfig() 
{
  wifi_ssid = default_ssid;
  wifi_pass = default_pass;
  name = default_name;

  SPIFFS.begin();
  File f = SPIFFS.open("/config.json", "r");
  if (!f) {
    return;
  }
  uint len = f.readBytes(config_buf, CONFIG_BUFFER_LEN - 1);
  //String content = configFile.readString();
  f.close();

  // HW_SERIAL.write(config_buf, len);
  // HW_SERIAL.println();
  config_buf[len] = 0; // make sure data is null terminated

  StaticJsonBuffer<256> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(config_buf);

  if (root.containsKey("ssid")) {
    wifi_ssid = root["ssid"];
  }
  if (root.containsKey("pass")) {
    wifi_pass = root["pass"];
  }
  if (root.containsKey("name")) {
    name = root["name"];
  }
  if (root.containsKey("baud")) {
    baud = root["baud"];
  } 
}

void setupWifi()
{
  WiFi.mode(WIFI_STA);

  // Set Hostname.
  String hostname(name);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);

  WiFi.begin(wifi_ssid, wifi_pass);

  while (WiFi.status() != WL_CONNECTED) { 
    wifi_log("Trying to connect to WIFI");
    delay(5000); 
  }
  delay(1000);

  IPAddress srvip = WiFi.localIP();
  wifi_log("WIFI connected", -1, srvip);

  if (!MDNS.begin(hostname.c_str(), srvip)) {
    wifi_log("Error setting up MDNS responder");
    while(1) { 
      delay(1000);
    }
  }
  wifi_log("mDNS responder started");
  
  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 8000);
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

#ifdef OTA
  // set up OTA server
  ArduinoOTA.setHostname(hostname.c_str());
  ArduinoOTA.onStart([]() { // switch off all the PWMs during upgrade
    wifi_log("OTA start");
  });
  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end
    wifi_log("OTA end");
  });
  ArduinoOTA.onError([](ota_error_t error) { 
    wifi_log("OTA error");
    ESP.restart(); 
  });
  ArduinoOTA.begin();
#endif
}
