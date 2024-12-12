#include <webserver.h>
#include "../../include/myDefines.h"
#include <webServer.h>



WebServer::WebServer(FS* fs) : server(80), events("/events"), filesystem(fs) {
    // Constructor body. Any additional setup code can go here.
}

WebServer& WebServer::getInstance(FS* fs) {
    static WebServer instance(fs); // Guaranteed to be destroyed and instantiated on first use
    return instance;
}

void WebServer::setup(MessageHandler &globalMessageHandler) {
  debugVariable++;
    setWifi();
    configRoutes(); 
    messageHandlerInstance = &globalMessageHandler;
    server.addHandler(&events);
    server.begin();
}


void WebServer::setWifi() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);  
}

void WebServer::end() {
  server.end();
}
void WebServer::begin() {
  server.begin();
}



void WebServer::configRoutes() {
  events.onConnect([this](AsyncEventSourceClient *client){ 
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    this->handleClientConnect(client);
  });
    server.onNotFound([this](AsyncWebServerRequest *request) {
        this->serveStaticFile(request);
    });

    server.on("/commandCalibrate", HTTP_GET, [this] (AsyncWebServerRequest *request){
      this->commandCalibrate(request);
    });
    server.on("/commandCalculate", HTTP_GET, [this] (AsyncWebServerRequest *request){
      this->commandCalculate(request);
    });
    server.on("/endCalibration", HTTP_GET, [this] (AsyncWebServerRequest *request){
      this->endCalibration(request);
    } );
    server.on("/commandAnimate", HTTP_GET, [this] (AsyncWebServerRequest *request){
      this->commandAnimate(request);
    });

    server.on("/goodNight", HTTP_GET, [this] (AsyncWebServerRequest *request){
      this->commandGoodNight(request);
    });
    server.on("/goodMorning", HTTP_GET, [this] (AsyncWebServerRequest *request){
      this->commandSetWakeup(request);
    });

    server.on("/submitPositions", HTTP_GET, [this] (AsyncWebServerRequest *request){

      this->submitPositions(request);
    });
    server.on("/setTime", HTTP_GET, [this] (AsyncWebServerRequest *request){
      this->setTime(request);
    });
    server.on("/testAnim", HTTP_GET, [this] (AsyncWebServerRequest *request){
      this->testAnim(request);
    });
    server.on("/neutral", HTTP_GET, [this] (AsyncWebServerRequest *request){
      this->setNeutral(request);
    });
    server.on("/triggerSync", HTTP_GET, [this] (AsyncWebServerRequest *request){
      this->triggerSync(request);
    });
    server.on("/sendSyncAsyncAnimation", HTTP_GET, [this] (AsyncWebServerRequest *request){
      this->sendSyncAsyncAnimation(request);
    });
        server.on("/updateStatus", HTTP_GET, [this] (AsyncWebServerRequest *request){
      this->statusUpdate(request);
    });
        server.on("/confirmClap", HTTP_GET, [this] (AsyncWebServerRequest *request){
      this->confirmClap(request);
    });
      server.on("/cancelClap", HTTP_GET, [this] (AsyncWebServerRequest *request){
    this->cancelClap(request);
    });
    server.on("/resetCalibration", HTTP_GET, [this] (AsyncWebServerRequest *request){
    this->resetCalibration(request);
    });
    server.on("/resetSystem", HTTP_GET, [this] (AsyncWebServerRequest *request){
    this->resetSystem(request);
    });
    server.on("/updateCalibrationStatus", HTTP_GET, [this] (AsyncWebServerRequest *request){
    this->updateCalibrationStatus(request);
    });
    server.on("/submitPdParams", HTTP_GET, [this] (AsyncWebServerRequest *request){ 
      this->submitPdParams(request);
    });   
     server.on("/setSyncAsyncParams", HTTP_GET, [this] (AsyncWebServerRequest *request){ 
      this->setSyncAsyncParams(request);
    });   

      server.on("/getParamsJson", HTTP_GET, [this] (AsyncWebServerRequest *request){
      String jsonString;
      jsonString = "TO FIX";
      //jsonString = messageHandlerInstance->getLedHandlerParams();
      request->send(200, "text/html", jsonString.c_str());
    });

}

void WebServer::handleClientConnect(AsyncEventSourceClient *client) {
    connected = true;
    client->send("");
}


void WebServer::commandCalibrate(AsyncWebServerRequest *request) {
}

void WebServer::updateCalibrationStatus(AsyncWebServerRequest *request) {
}

void WebServer::endCalibration(AsyncWebServerRequest *request) {
}
void WebServer::commandCalculate(AsyncWebServerRequest *request) {
}

void WebServer::commandAnimate(AsyncWebServerRequest *request) {
  String jsonString;
  message_animation animation;
  if (request->hasParam("brightness")) {
    int brightness = request->getParam("brightness")->value().toInt();
    animation.animationParams.strobe.brightness = brightness/255.0f;
  }
  jsonString = "{\"status\" : \"true\"}";
  request->send(200, "text/html", "{\"status\" : \"true\"}");
  }

void WebServer::setNeutral(AsyncWebServerRequest *request) {
    request->send(200, "text/html", "OK");
}

void WebServer::triggerSync(AsyncWebServerRequest *request) {
    request->send(200, "text/html", "OK");
}

void WebServer::serveStaticFile(AsyncWebServerRequest *request) {
  // Get the file path from the request
  String path = request->url();

  // Check if the file exists
  if (path == "/" || path == "/index.html") { // Modify this condition as needed
    path = "/addressList.html"; // Adjust the file path here
  }
  // Check if the file exists
  if (LittleFS.exists(path)) {
      // Open the file for reading
      File file = LittleFS.open(path, "r");
      if (file) {
      String contentType = "text/plain"; // Default Content-Type
      if (path.endsWith(".html") || path.endsWith(".htm")) {
        contentType = "text/html";
      } else if (path.endsWith(".css")) {
        contentType = "text/css";
      } else if (path.endsWith(".js")) {
        contentType = "application/javascript";
      } 
        // Read the contents of the file into a String
        String fileContent;
        while (file.available()) {
          fileContent += char(file.read());
        }

        // Close the file
        file.close();

        // Send the file content as response
        
        request->send(200, contentType, fileContent);
        return;
      }
  }

  // If file not found, send 404
  request->send(404, "text/plain", "File not found");
}

void WebServer::submitPositions(AsyncWebServerRequest *request) {
  float xpos = request->getParam("xpos")->value().toFloat();
  float ypos = request->getParam("ypos")->value().toFloat();
  float zpos = request->getParam("zpos")->value().toFloat();
  int boardId = request->getParam("boardId")->value().toInt();

  //messageHandler->setPositions(boardId, xpos, ypos, zpos);
  //rueberschieben
  //messageHandler->pushDataToSendQueue(MSG_SET_POSITIONS, -1);  
  request->send(200, "text/html", "OK");
}

void WebServer::confirmClap(AsyncWebServerRequest *request) {
  Serial.println("called ConfirmClap");
  if (request->hasParam("xpos") && request->hasParam("ypos") && request->hasParam("zpos") && request->hasParam("clapId")) {
      float xpos = request->getParam("xpos")->value().toFloat();
      float ypos = request->getParam("ypos")->value().toFloat();
      float zpos = request->getParam("zpos")->value().toFloat();
      int clapId = request->getParam("clapId")->value().toInt();
      Serial.println("calling msghandlerconfirmclap");
      String jsonString;
      jsonString = "{\"status\" : \"calibrationStatus\"}";
      request->send(200, "text/html", jsonString.c_str( ));

      // Process parameters
  } else {
      // Handle missing parameters
      request->send(400, "text/plain", "Missing parameters");
  }
  //messageHandler->setPositions(boardId, xpos, ypos, zpos);
  //rueberschieben
  //messageHandler->pushDataToSendQueue(MSG_SET_POSITIONS, -1);  
}
void WebServer::cancelClap(AsyncWebServerRequest *request) {
  int clapId = request->getParam("clapId")->value().toInt();
  request->send(200, "text/html", "{\"status\" : \"calibrationStatu\"}");
  //messageHandler->setPositions(boardId, xpos, ypos, zpos);
  //rueberschiebenudn z
  //messageHandler->pushDataToSendQueue(MSG_SET_POSITIONS, -1);  
}
void WebServer::resetCalibration(AsyncWebServerRequest *request) {
  request->send(200, "text/html", "{\"status\" : \"calibrationStatus\"}");

  //messageHandler->setPositions(boardId, xpos, ypos, zpos);
  //rueberschieben
  //messageHandler->pushDataToSendQueue(MSG_SET_POSITIONS, -1);  
}
void WebServer::setTime(AsyncWebServerRequest *request) {
  struct tm timeinfo;
  memset(&timeinfo, 0, sizeof(timeinfo));
  timeinfo.tm_year = request->getParam("year")->value().toInt() - 1900;
  timeinfo.tm_mon = request->getParam("month")->value().toInt() - 1;
  timeinfo.tm_mday = request->getParam("day")->value().toInt();
  timeinfo.tm_hour = request->getParam("hours")->value().toInt();
  timeinfo.tm_min = request->getParam("minutes")->value().toInt();
  timeinfo.tm_sec = request->getParam("seconds")->value().toInt();
  struct timeval tv;
  tv.tv_sec = mktime(&timeinfo);
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);
  request->send(200, "text/html", "OK");
}

void WebServer::testAnim(AsyncWebServerRequest *request) {
  message_data msg;
  msg.messageType = MSG_ANIMATION;
  memcpy (&msg.address, broadcastAddress, sizeof(broadcastAddress));
  msg.payload.animation.animationType = STROBE;
  msg.payload.animation.animationParams.strobe.brightness = 1.0f;
  msg.payload.animation.animationParams.strobe.duration = 3000;
  msg.payload.animation.animationParams.strobe.frequency = 15;
  msg.payload.animation.animationParams.strobe.hue = 0.0f;
  msg.payload.animation.animationParams.strobe.saturation = 0.0f;
  msg.payload.animation.animationParams.strobe.brightness = 1.0f;
  msg.payload.animation.animationParams.strobe.startTime = micros()+1000000;
  messageHandlerInstance->pushToSendQueue(msg);
  request->send(200, "text/html", "OK");
}
void WebServer::statusUpdate(AsyncWebServerRequest *request) {
  String returnString = "{\"status\":\"stateMachine->modeToText(stateMachine->getMode())\"}";
  request->send(200, "text/html", returnString.c_str());
  //events.send(stateMachine->modeToText(stateMachine->getMode()).c_str(), "statusUpdate");
}
void WebServer::statusUpdate() {
  String returnString = "{\"status\":\"stateMachine->modeToText(stateMachine->getMode())\"}";
  events.send(returnString.c_str(), "statusUpdate");
}

void WebServer::commandGoodNight(AsyncWebServerRequest *request) {
  int hours = request->getParam("hours")->value().toInt();
  int minutes = request->getParam("minutes")->value().toInt();
  int seconds = request->getParam("seconds")->value().toInt();
  Serial.println("going to sleep at "+String(hours)+":"+String(minutes)); 
  //messageHandler->pushDataToSendQueue(MSG_SET_SLEEP_WAKEUP, -1);
  request->send(200, "text/html", "OK");
}
void WebServer::commandSetWakeup(AsyncWebServerRequest *request) {
  int hours = request->getParam("hours")->value().toInt();
  int minutes = request->getParam("minutes")->value().toInt();
  int seconds = request->getParam("seconds")->value().toInt();
  request->send(200, "text/html", "OK");
}

void WebServer::sendSyncAsyncAnimation(AsyncWebServerRequest *request) {
  request->send(200, "text/html", "OK");
}

void WebServer::updateMode(String modeText) {
  Serial.println("switched to "+modeText);
    events.send(modeText.c_str(), "statusUpdate");
}


void WebServer::resetSystem(AsyncWebServerRequest *request) {
    ESP_LOGI("TBD", "Resetting system");  
}
void WebServer::submitPdParams(AsyncWebServerRequest *request) {

  //lag = request->getParam("lag")->value().toInt();
  //threshold = request->getParam("threshold")->value().toInt();
  //influence = request->getParam("influence")->value().toFloat();
  //Serial.println("called SubmitParams lag"+String(lag)+" threshold "+String(threshold)+" influence "+String(influence));
  PdParamsChanged = true;
  request->send(200, "text/html", "OK");
}

void WebServer::setSyncAsyncParams(AsyncWebServerRequest *request) {
  int minS, maxS, minP, maxP, minR, maxR, minSp, maxSp;
  int minRed, maxRed, minGreen, maxGreen, minBlue, maxBlue;
  int minAniReps, maxAniReps;
  if (request->hasParam("minSpeed")) {
      minS = request->getParam("minSpeed")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: minSpeed");
      return;
  }

  if (request->hasParam("maxSpeed")) {
      maxS = request->getParam("maxSpeed")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: maxSpeed");
      return;
  }

  if (request->hasParam("minPause")) {
      minP = request->getParam("minPause")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: minPause");
      return;
  }

  if (request->hasParam("maxPause")) {
      maxP = request->getParam("maxPause")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: maxPause");
      return;
  }

  if (request->hasParam("minReps")) {
      minR = request->getParam("minReps")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: minReps");
      return;
  }

  if (request->hasParam("maxReps")) {
      maxR = request->getParam("maxReps")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: maxReps");
      return;
  }

  if (request->hasParam("minSpread")) {
      minSp = request->getParam("minSpread")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: minSpread");
      return;
  }

  if (request->hasParam("maxSpread")) {
      maxSp = request->getParam("maxSpread")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: maxSpread");
      return;
  }

  if (request->hasParam("minRed")) {
      minRed = request->getParam("minRed")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: minRed");
      return;
  }

  if (request->hasParam("maxRed")) {
      maxRed = request->getParam("maxRed")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: maxRed");
      return;
  }

  if (request->hasParam("minGreen")) {
      minGreen = request->getParam("minGreen")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: minGreen");
      return;
  }

  if (request->hasParam("maxGreen")) {
      maxGreen = request->getParam("maxGreen")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: maxGreen");
      return;
  }

  if (request->hasParam("minBlue")) {
      minBlue = request->getParam("minBlue")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: minBlue");
      return;
  }

  if (request->hasParam("maxBlue")) {
      maxBlue = request->getParam("maxBlue")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: maxBlue");
      return;
  }

  if (request->hasParam("minAniReps")) {
      minAniReps = request->getParam("minAniReps")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: minAniReps");
      return;
  }

  if (request->hasParam("maxAniReps")) {
      maxAniReps = request->getParam("maxAniReps")->value().toInt();
  } else {
      request->send(400, "text/plain", "Missing parameter: maxAniReps");
      return;
  }

  //messageHandler->setSyncAsyncParams(minS, maxS, minP, maxP, minSp, maxSp, minR, maxR,minAniReps, maxAniReps, minRed, maxRed, minGreen, maxGreen, minBlue, maxBlue );
  request->send(200, "text/html", "OK");


}