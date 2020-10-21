// serial communication from Uno to NodeMCU
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <OneWire.h> 
#include <DallasTemperature.h>

#define turbidityPin A0 // analog pin A0 for turbidity
#define phPin A1 // analog pin A1 for pH

SoftwareSerial linkSerial(10, 11); // Rx, Tx for Uno

//temperature values
const int temperaturePin = 2; //DS18S20 Signal pin on digital pin 2
OneWire tempWire(temperaturePin); 
DallasTemperature tempSensor(&tempWire); // pass tempWire to Dallas Temperature Library

void setup() {
  // Initialize "debug" serial port
  Serial.begin(115200);
  while (!Serial) continue;

  // Initialize the "link" serial port
  linkSerial.begin(4800);
}
 
void loop() {
  // Values  to transmit over serial line
  float pHValue = getpH();
  float turbidity = getTurbidity();
  float temperature = getTemp();
  
  // Print the values on the "debug" serial port
   Serial.println(pHValue, 2);
   Serial.println(turbidity);
   Serial.println(temperature);
  
  // Create the JSON document
  const size_t CAPACITY = JSON_OBJECT_SIZE(3) + 25;
  StaticJsonDocument<CAPACITY> doc;
  doc["ph"] = pHValue;
  doc["turbidity"] = turbidity;
  doc["temperature"] = temperature;

  // Send the JSON document over the "link" serial port
  serializeJson(doc, linkSerial);

  // 5 second delay
  delay(5000);
}

float getTurbidity()
{
  float ntu;
  int turbidityValue = analogRead(turbidityPin);// read the input on analog pin
  float voltage = turbidityValue * (5.0 / 1024.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):

  if(voltage < 2.7){
      ntu = 3000;
    }else{
      ntu = (-1120.4 * sq(voltage)) + (5742.3 * voltage) - 4352.9; 
    }
  
  return ntu;
}

float getpH()
{
  unsigned long int avgValue;  //Store the average value of the sensor feedback
  float b;
  int buf[10],temp;

  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(phPin);
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


float getTemp() 
{
    tempSensor.requestTemperatures(); // Send the command to get temperature readings 
    return tempSensor.getTempCByIndex(0); 
}
