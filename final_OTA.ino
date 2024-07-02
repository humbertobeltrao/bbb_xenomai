#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <ESPAsyncWebServer.h>
#include <WiFiClientSecure.h>
#include "cert.h"

const char * ssid = "";
const char * wifiPassword = "";
int status = WL_IDLE_STATUS;
//int incomingByte;

#ifdef __cplusplus
  extern "C" {
 #endif

  uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

uint8_t temprature_sens_read();



AsyncWebServer server_health(8888);

WiFiServer server(8080);  // Use any available port

int LED2 = 2;

String firmVersion = "";

//String FirmwareVer = {
//    "4.0"
//};

//#define URL_fw_Version "https://raw.githubusercontent.com/humbertobeltrao/esp32/main/esp32/bin_version.txt"
//#define URL_fw_Bin "https://raw.githubusercontent.com/humbertobeltrao/esp32/main/esp32/fw.bin"

void setup() {
    pinMode(LED2, OUTPUT);
//    Serial.print("Active Firmware Version:");
//    Serial.println(FirmwareVer);
    Serial.begin(115200);

    WiFi.begin(ssid, wifiPassword);

    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");

        int i = 0;
        if (i == 10) {
            ESP.restart();
        }
        i++;
    }
    Serial.println(WiFi.localIP());
    

    server.begin();
    Serial.println("Server listening on port 8080");

     // Add a new route for health status on the AsyncWebServer
     server_health.on("/health", HTTP_GET, [](AsyncWebServerRequest *request) {
        // Enable CORS by adding appropriate headers
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", getHealthStatus());
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    server_health.begin();
    Serial.println("HTTP Server listening on port 8888");
}

void loop() {

    delay(1000);
//    Serial.print(" Active Firmware Version:");
    //Serial.println(FirmwareVer);
    digitalWrite(LED2, HIGH);
    delay(5000);
    digitalWrite(LED2, LOW);
    delay(5000);
    
    if (WiFi.status() != WL_CONNECTED) {
        reconnect();
    }
    WiFiClient client = server.available();
    if (client) {
      Serial.println("New client connected");
      // Read data from the client
      String fw_version = client.readStringUntil('\n');
      Serial.println(fw_version);
      String fw_url = client.readStringUntil('\n');
      Serial.println(fw_url);
 
      
      if(firmwareVersionCheck(fw_version)) {
        firmwareUpdate(fw_url);
        
        
      }

      client.stop();  
  
      // Close the connection
      //client.stop();
      //Serial.println("Client disconnected");
    }
//    server.on("/new_version", HTTP_GET, [] (AsyncWebServerRequest *request) {
//    String firmwareURL = request->getParam("url", true)->value();
//    if(!firmwareURL.equals("")) {
//      Serial.println("Firmware Update In Progress...");
//      Serial.println(firmwareURL);
//      firmwareUpdate(firmwareURL);
//    } else {
//      Serial.println("No firmware URL received!");
//    }
//  });

//    if (Serial.available() > 0) {
//        incomingByte = Serial.read();
//        if (incomingByte == 'U') {
//            Serial.println("Firmware Update In Progress..");
//            if (FirmwareVersionCheck()) {
//                firmwareUpdate();
//            }
//        }
//    }
  
}

void reconnect() {
    int i = 0;
    // Loop until we're reconnected
    status = WiFi.status();
    if (status != WL_CONNECTED) {
        WiFi.begin(ssid, wifiPassword);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
            if (i == 10) {
                ESP.restart();
            }
            i++;
        }
        Serial.println("Connected to AP");
    }
}

void firmwareUpdate(const String& firmwareURL) {
    
    WiFiClientSecure client;
    client.setCACert(rootCACertificate);
    t_httpUpdate_return ret = httpUpdate.update(client, firmwareURL);
    Serial.println(ret);
    switch (ret) {
    case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;

    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

    case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
}

int firmwareVersionCheck(const String& firmwareVersion) {
    String payload;
    int httpCode;
    String FirmwareURL = "";
    FirmwareURL += firmwareVersion;
    FirmwareURL += "?";
    FirmwareURL += String(rand());
    Serial.println(FirmwareURL);
    WiFiClientSecure * client = new WiFiClientSecure;

    if (client) {
        client -> setCACert(rootCACertificate);
        HTTPClient https;

        if (https.begin( * client, FirmwareURL)) {
            Serial.print("[HTTPS] GET...\n");
            // start connection and send HTTP header
            
            delay(100);
            httpCode = https.GET();
            delay(100);
            if (httpCode == HTTP_CODE_OK) // if version received
            {
                payload = https.getString(); // save received version
            } else {
                Serial.print("Error Occured During Version Check: ");
                Serial.println(httpCode);
            }
            https.end();
        }
        delete client;
    }

    if (httpCode == HTTP_CODE_OK) // if version received
    {
        payload.trim();
        if (payload.equals(firmwareVersion)) {
            Serial.printf("\nDevice  IS Already on Latest Firmware Version:%s\n", firmwareVersion);
            return 0;
        } else {
            Serial.println(payload);
            Serial.println("New Firmware Detected");
            return 1;
        }
    }
    return 0;
}

String getHealthStatus() {
    String healthStatus = "";
    healthStatus += "Internal temperature: " + String((temprature_sens_read() - 32) / 1.8) + "°C"+"\n";
    //healthStatus += "Firmware version: " + String(firmVersion) + "\n";

    return healthStatus;
}
