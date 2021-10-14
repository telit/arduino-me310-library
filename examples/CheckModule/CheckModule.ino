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
    In this example sketch, it is shown how to use a basic AT commands via ME310 library.\n

  @version
    1.0.0

  @note

  @author


  @date
    20/10/2020
 */

#include <ME310.h>

/*
    //sercom2
    #define PIN_MODULE_RX 29
    #define PIN_MODULE_TX 28
    #define PAD_MODULE_TX UART_TX_PAD_0
    #define PAD_MODULE_RX SERCOM_RX_PAD_1
    #define PIN_MODULE_RTS 30
    #define PIN_MODULE_CTS 31
 */

#ifndef ARDUINO_TELIT_SAMD_CHARLIE
#define ON_OFF 6 /*Select the right GPIO to control ON_OFF*/
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
bool ready = false;

void setup() {
  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  myME310.begin(115200);

  delay(1000);
  myME310.powerOn(ON_OFF);
  if(myME310.attention() == ME310::RETURN_VALID)
  {
	 ready = true;
  }
}

void loop() {
	if(ready)
	{
		digitalWrite(LED_BUILTIN, HIGH);
		delay(500);
		digitalWrite(LED_BUILTIN, LOW);
		delay(500);
	}
	else
	{
		exit(0);
	}
}
