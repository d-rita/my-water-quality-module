#include <OneWire.h>
#include <ThingSpeak.h>
#include <ESP8266WiFi.h>
#include "secrets.h"

// sensor pins

// temperature
const int temperaturePin = D2; //DS18S20 Signal pin on digital 2
OneWire ds(temperaturePin);  // Temperature chip i/o

// analog pin
const int SensorPin = A0; // analog pin A0 for turbidity and pH sensors

int pin1 = D0;      // enable reading pH sensor 
int pin2 = D1;      // enable reading turbidity sensor


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

void setup(void)
{
  Serial.begin(9600);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  connectToWiFi();
}

void loop(void)
{
//collect sensor values: temperature, pH, Turbidity 
  float temperature = getTemp();
  Serial.println(temperature);

  delay(100);
  
  digitalWrite(pin1, HIGH);
  float pHValue = getpH();
  Serial.println(pHValue, 2);
  digitalWrite(pin1, LOW);

  delay(100);

  digitalWrite(pin2, HIGH);
  float turbidity = getTurbidity();
  Serial.println(turbidity);
  digitalWrite(pin2, LOW);

 
//  add data to TS
  writeMultipleTSData(channelID, pHField, (pHValue, 2), turbidityField, turbidity, tempField, temperature);
  
//  insert a time delay for every 5-10 mins during network upload
  delay(20000); //20 sec delay
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


float getpH()
{
  unsigned long int avgValue;  //Store the average value of the sensor feedback
  float b;
  int buf[10],temp;

  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(SensorPin);
    delay(10);
  }
  
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
    
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;                      //convert the millivolt into pH value
  return phValue;
}


float getTurbidity()
{
  float ntu;
  int turbidityValue = analogRead(SensorPin);// read the input on analog pin 0:
  float voltage = turbidityValue * (5.0 / 1024.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):

  if(voltage < 2.7){
      ntu = 3000;
    }else{
      ntu = (-1120.4 * sq(voltage)) + (5742.3 * voltage) - 4352.9; 
    }
  
  return ntu; // print out the value you read:
}

float getTemp() //returns the temperature from one DS18S20 in DEG Celsius
{
  byte data[12];
  byte addr[8];

  if ( !ds.search(addr))
  {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7])
  {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28)
  {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++)
  { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum; 
}
