#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include "secrets.h"

SoftwareSerial linkSerial(D6, D5); // RX, TX for nodemcu

// network parameters
const char* ssid = MY_SSID;
const char* password = PASSWORD;

// thingspeak information
char thingSpeakAddress[] = "api.thingspeak.com";

unsigned long channelID = CHANNEL_ID;

char* readAPIKey = READ_APIKEY;
char* writeAPIKey = WRITE_APIKEY;

//fields
unsigned int pHField = 1;
unsigned int turbidityField = 2;
unsigned int tempField = 3;

WiFiClient client;

void setup() {
  // Initialize "debug" serial port
  Serial.begin(115200);
  while (!Serial) continue;

  // Initialize the "link" serial port
  linkSerial.begin(4800);

  connectToWiFi();
}
 
void loop() {
  // Check if the other Arduino is transmitting
  if (linkSerial.available()) 
  {
    // Allocate the JSON document
    StaticJsonDocument<400> doc;

    // Read the JSON document from the "link" serial port
    DeserializationError err = deserializeJson(doc, linkSerial);

    if (err == DeserializationError::Ok) 
    {
      // Print the values
      // (we must use as<T>() to resolve the ambiguity)
      Serial.print("pH = ");
      Serial.println(doc["ph"].as<float>());
      Serial.print("turbidity = ");
      Serial.println(doc["turbidity"].as<float>());
      Serial.print("temperature = ");
      Serial.println(doc["temperature"].as<float>());

      // values 
      float pH = doc["ph"];
      float turbidity = doc["turbidity"];
      float temperature = doc["temperature"];

    //  add data to TS
      writeMultipleTSData(channelID, pHField, pH, turbidityField, turbidity, tempField, temperature);
  
    delay(5000); //insert a time delay for every 5-10 mins during network upload
    } 
    else 
    {
      // Print error to the "debug" serial port
      Serial.print("deserializeJson() returned ");
      Serial.println(err.c_str());
  
      // Flush all bytes in the "link" serial port buffer
      while (linkSerial.available() > 0)
        linkSerial.read();
    }
  }
}

int connectToWiFi()
{
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi... ");
  
  while(WiFi.status() != WL_CONNECTED) {  
    delay(2500);
    Serial.println(".");
  }
  Serial.println();

  Serial.print("Connected to this IP Address: ");
  Serial.println(WiFi.localIP());
  
  ThingSpeak.begin(client);
}


int writeMultipleTSData(long TSChannel, unsigned int TSField1, float data1, unsigned int TSField2, float data2, unsigned int TSField3, float data3)
{
  ThingSpeak.setField(TSField1, data1);
  ThingSpeak.setField(TSField2, data2);
  ThingSpeak.setField(TSField3, data3);
  
  int writeSuccess = ThingSpeak.writeFields(TSChannel, writeAPIKey); // write data to channel
  if (writeSuccess) {
    Serial.println("ThingSpeak channel successfully updated.");
  }
}
