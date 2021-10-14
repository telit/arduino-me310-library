/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    ME310.cpp
    string.h
    stdio.h

  @brief
    Driver Library for ME310 Telit Modem

  @details
    The library contains a single class that implements a C++ interface to all ME310 AT Commands.\n
    It makes it easy to build Arduino applications that use the full power of ME310 module.\n
	
  @version
    1.0.0
  
  @note

  @author
    Cristina Desogus

  @date
    28/10/2020
 */

#include <ME310.h>

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
ME310::return_t rc;     //Enum of return value  methods


void setup() {
  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  myME310.begin(115200);
  delay(1000);

  Serial.println("TURN ON ME310");
  myME310.powerOn();
  Serial.println("ME310 TURNED ON");
  Serial.println("Bridge Communication Enabled");

}

void loop() {
  while (SerialModule.available()) {
    Serial.write(SerialModule.read());
  }
  while (Serial.available()) {
    SerialModule.write(Serial.read());
  }
}
