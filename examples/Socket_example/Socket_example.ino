/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    ME310.cpp
    string.h
    stdio.h

  @brief
    Sample test of using sockets via AT commands

  @details
    In this example sketch, the use of sockets is shown through the commands offered by the ME310 library.\n   
	NOTE:\n
	For correct operation it is necessary to set the correct APN.

  @version 
    1.0.0
  
  @note

  @author
    Cristina Desogus

  @date
    26/02/2021
 */
#include <ME310.h>
//#include <string.h>

#define APN "apn"

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
int connID = 1;         //Socket connection identifier.
char ipProt[] = "IP";   //Packet Data Protocol type

char server[] = "modules.telit.com";    //echo server
int port = 10510;

void setup() {

  int rc;

  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  myME310.begin(115200);

  delay(3000);
  myME310.module_reboot();                            // issue command at#reboot
  
  delay(10000);
  Serial.println("Telit Test AT Socket command");
  myME310.powerOn();
  Serial.println("ME310 ON");

  myME310.report_mobile_equipment_error(2);           //issue command AT+CMEE=2 and wait for answer or timeout
  myME310.read_enter_pin();                           //issue command AT+pin? in read mode, check that the SIM is inserted and the module is not waiting for the PIN
  if(strcmp(myME310.buffer_cstr(2), "OK") == 0)       // read response in 2 array position
  {
    Serial.println("Define PDP Context");
    rc = myME310.define_pdp_context(cID, ipProt, APN);    //issue command AT+CGDCONT=cid,PDP_type,APN
    if(rc == ME310::RETURN_VALID)
    {
      myME310.read_define_pdp_context();                  //issue command AT+CGDCONT=? (read mode)
      Serial.print("pdp context read: ");
      Serial.println(myME310.buffer_cstr(1));              //print second line of modem answer

      Serial.print("gprs network registration status: ");
      rc = myME310.read_gprs_network_registration_status();  //issue command AT+CGREG=? (read mode) 
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
    }
  }
}

void loop() {

  char data[] = "We are sending some data\nSome data OK\nAnother data";

  Serial.print("Socket configuration: ");
  ME310::return_t r = myME310.socket_configuration(connID, cID);         //issue command AT#SCFG=connID,cID and wait for answer or timeout
  Serial.println(myME310.return_string(r));                              //returns a string with return_t codes
  if(r == ME310::RETURN_VALID)
  {
    Serial.print("Socket dial: ");
    r = myME310.socket_dial(connID, 0, port, server, 0, 0, 1, 0, 0, ME310::TOUT_1MIN);   // issue commandAT#SD=connID,protocol,port,IPAddrServer,timeout
    Serial.println(myME310.return_string(r));                           //returns a string with return_t codes
    if(r == ME310::RETURN_VALID)
    {
      delay(100);
      Serial.print("Socket Status: ");
      r = myME310.socket_status(connID, ME310::TOUT_10SEC);             //issue command AT#SS=connID and wait for answer or timeout
      delay(100);
      Serial.println(myME310.return_string(r)); 
      
      Serial.print("SEND: ");
      r = myME310.socket_send_data_command_mode_extended(connID, (int) sizeof(data), data, 1, ME310::TOUT_30SEC);
      Serial.println(myME310.return_string(r));
      if(r != ME310::RETURN_VALID)
      {
        Serial.println("Send is failed");
      }
      else
      {
        Serial.print("Socket Listen: ");
        r = myME310.socket_listen(connID, 0, port);                     //issue command AT#SL=connID,listenState(0 close socket listening),port and wait for answer or timeout
        Serial.println(myME310.return_string(r));
        delay(5000);
        if(r == ME310::RETURN_VALID)
        {
          Serial.print("READ: ");
          r = myME310.socket_receive_data_command_mode(connID, (int) sizeof(data), 0, ME310::TOUT_10SEC); //issue command AT#SRECV=connID,size and wait for answer or timeout
          Serial.println(myME310.return_string(r));
          if(r == ME310::RETURN_VALID)
          {
            Serial.print("Payload: <");
            Serial.print((String)myME310.buffer_cstr_raw());  //  print modem answer in raw mode
            Serial.println(">");
          }
          else
          {
            Serial.println(myME310.return_string(r));   
          }
          r = myME310.socket_listen(connID, 0, port);
        }
        else
        {
          Serial.println(myME310.return_string(r));
        }
      }
    }
    else
    {
      Serial.println(myME310.return_string(r));
    } 
  }
  Serial.println("The application has ended...");
  exit(0);
}
