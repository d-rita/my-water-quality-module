#### My Water Monitoring App

This project makes use of:
- Analog Turbidty Sensor
- Analog pH meter pro
- DS18B20 Temperature Sensor
- NodeMCU
- Arduino uno

This project utilises the Arduino IDE and the above components. 

Version 1 utilises only a NodeMCU as both the AP and MCU as well. 

Version 2 utlises the Arduino uno as the MCU and the NodeMCU as an AP

Note: 
- All secret values like API keys passwords are contained in a secrets.h file to prevent public access to them.
- When running on the IDE, the relevant libraries have to be installed, and the correct board as well as port selected.
