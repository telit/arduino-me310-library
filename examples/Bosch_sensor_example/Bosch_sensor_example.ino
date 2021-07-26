/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    ME310.cpp
    string.h
    stdio.h

  @brief
    Sample test of the use of AT commands via ME310 library

  @details
    In this example sketch, it is shown how to use BMA400 library of management the BMA400 accelerometer from Bosch Sensortec.
    To run this sketch, it is necessary to download the Grove_3Axis_Digital_Accelerometer_BMA400 library from
    https://github.com/Seeed-Studio/Grove_3Axis_Digital_Accelerometer_BMA400.

  @version 
    2.0.0
  
  @note

  @author
    Cristina Desogus

  @date
    29/04/2021
 */

#include <BMA400.h>

float x = 0, y = 0, z = 0;
float xACC = 0, yACC = 0, zACC = 0; 
int16_t temperature = 0;
uint8_t idDev = 0; 

void setup() {

  Wire.begin();
  Serial.begin(115200);
  delay(1000);
  Serial.println("Telit BMA400 test ");
  
  Serial.println("Checking BMA400 connection");
  delay(1000);
  if(bma400.isConnection())  //Control of BMA400 connection is ON
  {
    bma400.initialize();
    Serial.println("BMA400 is connected");
  }
  else
  {
    Serial.println("BMA400 is not connected");
  }
  delay(2000);  
  idDev = bma400.getDeviceID();  // Get id Device
  Serial.println((String)"Device id: " + idDev);
}

void loop() {
  bma400.getAcceleration(&x, &y, &z); // Get acceleration values x, y, z by reference
  temperature = bma400.getTemperature(); // Get temperature value
  xACC = bma400.getAccelerationX();  // Get acceleration value X
  yACC = bma400.getAccelerationY();  // Get acceleration value Y
  zACC = bma400.getAccelerationZ(); // Get acceleration value Z
  Serial.println("Accelation values by getAcceleration function: ");
  Serial.println((String)"X-> " + x + ", Y-> "+ y+ ", Z-> " +z);
  Serial.println("Accelation value x by getAccelerationX : ");
  Serial.println((String)"X-> " + xACC);
  Serial.println("Accelation value y by getAccelerationY : ");
  Serial.println((String)"Y-> "+ yACC);
  Serial.println("Accelation value z by getAccelerationZ : ");
  Serial.println((String)"Z-> " +zACC);
  Serial.println((String)"Temperature: " + temperature);
  delay(3000);
}
