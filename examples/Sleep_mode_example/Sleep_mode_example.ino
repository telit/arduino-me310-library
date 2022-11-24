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
    In this example sketch, it is shown how to get the board to sleep mode using the AT + CFUN = 5 command via the ME310 library.\n

  @version
    1.0.0

  @note

  @author
    Cristina Desogus

  @date
    04/20/2022
 */


#include <ME310.h>
#include <string.h>

/*When NMEA_DEBUG is 0 Unsolicited NMEA is disable*/
#define NMEA_DEBUG 0

#ifndef ARDUINO_TELIT_SAMD_CHARLIE
#define ON_OFF 6 /*Select the GPIO to control ON_OFF*/
#endif

using namespace me310;

/*
 * If a Telit-Board Charlie is not in use, the ME310 class needs the Uart Serial instance in the constructor, that will be used to communicate with the modem.\n
 * Please refer to your board configuration in variant.h file.
 * Example:
 * Uart Serial1(&sercom4, PIN_MODULE_RX, PIN_MODULE_TX, PAD_MODULE_RX, PAD_MODULE_TX, PIN_MODULE_RTS, PIN_MODULE_CTS);
 * ME310 myME310 (Serial1);
 */

ME310 myME310;
ME310::return_t rc;

int val;

void setup()
{
  Serial.begin(115200);
  myME310.begin(115200);

  delay(4000);
  myME310.powerOn();
  Serial.println("ME310 is ON");

  Serial.println("Application start");
  delay(1000);
  Serial.print("Set DTR PIN: ");
  rc = myME310.gpio_control(5,0,9,1);   //this command sets GPIO5 as DTR and save the configuration
  Serial.println(myME310.return_string(rc));

  /*After the AT#GPIO=5,0,9,1 command, the module must be rebooted.*/

  myME310.module_reboot();    //issue command AT#REBOOT
  delay(5000);

  pinMode(PIN_MODULE_DTR,OUTPUT);     //this command sets the DTR PIN in OUTPUT mode
  digitalWrite(PIN_MODULE_DTR, LOW);  //this command sets the DTR PIN to LOW level
}

void loop() {

  Serial.print("Value of DTR is: ");
  val = digitalRead(PIN_MODULE_DTR);    //this command reads the DTR PIN value, it is only for control
  Serial.println(val);

  Serial.print("CFUN=5 result: ");
  rc = myME310.set_phone_functionality(5);  //issues AT#CFUN=5 command and wait for answer or timeout
  Serial.println(myME310.return_string(rc));

  myME310.attention();  //issue AT command, for control
  Serial.print("Control AT command result: ");
  if(myME310.buffer_cstr(1) != NULL)
  {
    Serial.println(myME310.buffer_cstr(1));
  }

  delay(3000);
  Serial.println("Move DTR to OFF");
  digitalWrite(PIN_MODULE_DTR, HIGH);   //this command sets the DTR PIN to HIGH level

  Serial.print("Value of DTR: ");
  val = digitalRead(PIN_MODULE_DTR);
  Serial.println(val);

  myME310.attention();
  if(myME310.buffer_cstr(1) == NULL)
  {
    Serial.println("Module is in sleep mode");
  }
  else
  {
    Serial.println("ERROR module is not in sleep mode");
  }

  delay(1000);
  Serial.println("Move DTR to ON");
  digitalWrite(PIN_MODULE_DTR, LOW);  //this command sets the DTR PIN to LOW level, it moves the DTR PIN to wake-up the module

  Serial.print("Value of DTR: ");
  val = digitalRead(PIN_MODULE_DTR);
  Serial.println(val);
  delay(3000);

  myME310.attention();
  if(myME310.buffer_cstr(1) != NULL)
  {
    Serial.println("Module is awakened");
  }
  else
  {
    Serial.println("ERROR Module is not awakened");
  }
  exit(0);

}