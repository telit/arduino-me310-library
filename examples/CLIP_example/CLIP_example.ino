/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    ME310.cpp
    string.h
    stdio.h

  @brief
    Sample test of the use of AT+CLIP command via ME310 library

  @details
    In this example sketch, the use of methods offered by the ME310 library for using AT commands is shown.\n
    The Calling line identify is enable and wait_for_unsolicited is call in loop part. The response is printed with buffer_cstr_raw.\n
    To use this example is necessary a voice variant firmware and a SIM card that supports voice.

  @version
    1.0.0

  @note

  @author
    Cristina Desogus

  @date
    11/08/2021
 */

#include <ME310.h>
#include <string.h>

#define APN "APN"

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

int cID = 1;            //PDP Context Identifier
char ipProt[] = "IP";   //Packet Data Protocol type
bool flag;


void setup() {

  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(ON_OFF, LOW);

  Serial.begin(115200);
  myME310.begin(115200);

  delay(2000);
  Serial.println("Telit Test AT CLIP command");
  myME310.powerOn(ON_OFF);
  Serial.println("ME310 ON");
  delay(2000);
  Serial.println("AT Command");
  myME310.report_mobile_equipment_error(2);           //issue command AT+CMEE=2 and wait for answer or timeout
  rc = myME310.read_enter_pin();                            //issue command AT+cpin? in read mode, check that the SIM is inserted and the module is not waiting for the PIN
  if(strcmp(myME310.buffer_cstr(2), "OK") == 0)        // read response in 2 array position
  {
    delay(2000);
    Serial.println("Define PDP Context");
    rc = myME310.define_pdp_context(cID, ipProt, APN);   //issue command AT+CGDCONT=cid,PDP_type,APN
    if(rc == ME310::RETURN_VALID)
    {
      myME310.read_define_pdp_context();              //issue command AT+CGDCONT=? (read mode)
      Serial.print("pdp context read :");
      Serial.println(myME310.buffer_cstr(1));              //print second line of modem answer

      Serial.print("gprs network registration status :");
      rc = myME310.read_gprs_network_registration_status();  //issue command AT+CGREG? (read mode)
      Serial.println(myME310.buffer_cstr(1));
      if(rc == ME310::RETURN_VALID)
      {
          while ((strcmp(myME310.buffer_cstr(1), "+CGREG: 0,1") != 0) &&  (strcmp(myME310.buffer_cstr(1), "+CGREG: 0,5") != 0))
          {
              delay(3000);
              myME310.read_gprs_network_registration_status();
              Serial.println(myME310.buffer_cstr(1));
          }
      }
      Serial.println("Activate context");
      myME310.context_activation(cID, 1);        //issue command AT#SGACT=cid,state and wait for answer or timeout

      rc = myME310.calling_line_identification_presentation(1);   //issue command AT+CLIP=enable and wait for answer or timeout
      Serial.println(rc);
      if(rc == ME310::RETURN_VALID)
      {
        Serial.println("Calling Line Identity is enable");
        flag = true;
      }
      else
      {
        Serial.println((String)"Error: " + rc + " Error string: " + myME310.buffer_cstr(2));
        flag = false;
      }
    }
  }
  else
  {
      Serial.println((String)"Error: " + rc + " Error string: " + myME310.buffer_cstr(2));
  }
}

void loop() {

  if(flag)
  {
    myME310.wait_for_unsolicited(ME310::TOUT_5SEC);
    Serial.println(myME310.buffer_cstr_raw());
    delay(1000);
  }
  else
  {
    Serial.println("Calling Line Identity is not enable");
    exit(0);
  }
}
