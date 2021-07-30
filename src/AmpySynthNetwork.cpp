#include "AmpySynthNetwork.h"


void AmpySynthNetwork::init()
{


  if(!SPIFFS.begin(true)){
     Serial.println("An Error has occurred while mounting SPIFFS");
  }

  //Set up web config / update service
  iotWebConf.setWifiConnectionCallback([this]{ _wifiConnected(); });
  iotWebConf.setupUpdateServer(
    [this](const char* updatePath) { updateServer.setup(&web, updatePath); },
    [this](const char* userName, char* password) { updateServer.updateCredentials(userName, password); });
    iotWebConf.setConfigPin(_resetPin);
  iotWebConf.skipApStartup();
  iotWebConf.init();
  
  web.on("/", [this]{ _handleRoot(); });
  web.on("/config", [this]{ iotWebConf.handleConfig(); });
  web.onNotFound([this](){ iotWebConf.handleNotFound(); });


  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {
        type = "filesystem";
        SPIFFS.end();
      }

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  
}

void AmpySynthNetwork::loop()
{
  iotWebConf.doLoop();
  ArduinoOTA.handle();
  ws.loop();
}

void AmpySynthNetwork::_wifiConnected()
{
  Serial.println("Wifi connected");
  ws.begin();
  //_ws.onEvent(webSocketEvent);
  ArduinoOTA.begin();
}

void AmpySynthNetwork::_handleRoot()
{
  if (iotWebConf.handleCaptivePortal()) {
    return;
  }
  File file = SPIFFS.open("/index.html", FILE_READ);
  Serial.println(file.size()); 
  if (!file) {
      Serial.println("file open failed");
      web.send(404, "Missing FS, upload FS image");
  } 
  if (web.streamFile(file, "text/html") != file.size()) {
    Serial.println("Sent less data than expected!");
  }else{
      Serial.println("Page served!");
  }
  file.close();
}