/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    ME310.cpp
    string.h
    stdio.h

  @brief
    Sample test of the use of AT#PING command via ME310 library

  @details
    In this example sketch, the use of methods offered by the ME310 library for using AT commands is shown.\n
    The PDP context is defined, the registration status read, the context activated and finally the ping command is sent and the response printed.\n
    

  @version 
    1.0.0
  
  @note

  @author
    Cristina Desogus

  @date
    26/02/2021
 */
#include <ME310.h>
#include <string.h>

using namespace me310;

#define APN "nbiot.tids.tim.it"

char server[] = "8.8.8.8";

ME310 myME310;

void turnOnModule (){
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  
  while(myME310.attention() == ME310::RETURN_TOUT)
   {
      digitalWrite(ON_OFF, HIGH);  
      digitalWrite(LED_BUILTIN, HIGH); 
      delay(6000);                      
      digitalWrite(ON_OFF, LOW);
      digitalWrite(LED_BUILTIN, LOW);    
      delay(1000);                      
   }
}


void setup() {

  int cID = 1;            //PDP Context Identifier
  char ipProt[] = "IP";   //Packet Data Protocol type
  ME310::return_t rc;     //return value

  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(ON_OFF, LOW);

  Serial.begin(115200);
  myME310.begin(115200);

  delay(1000);
  Serial.println("Telit Test AT command Ping");
  turnOnModule();
  Serial.println("ME310 is ON");

  myME310.report_mobile_equipment_error(2);           //issue command AT+CMEE=2 and wait for answer or timeout
  Serial.println("Define PDP Context");
  rc = myME310.define_pdp_context(cID, ipProt, APN);  //issue command AT+CGDCONT=cid,PDP_type,APN
  if(rc == ME310::RETURN_VALID)
  {
    myME310.read_define_pdp_context();                //issue command AT+CGDCONT=? (read mode)
    Serial.print("pdp context read :");
    Serial.println(myME310.buffer_cstr(1));           //print second line of modem answer

    Serial.print("gprs network registration status :");
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

void loop() {
  ME310::return_t rc;
  rc = myME310.ping(server, ME310::TOUT_20SEC);        //issue command AT#PING=server and wait for answer or timeout (ME310::TOUT_30SEC is 30 seconds)
  Serial.println((String)"Ping to server: " + server);
  if(rc == ME310::RETURN_VALID)
  {
    Serial.println("Ping value:<");
    int i = 0;
    while(myME310.buffer_cstr(i) != NULL)
    {
      delay(1000);
      Serial.print(myME310.buffer_cstr(i));
      i++;
    }
    Serial.println(">");
    Serial.println((String) "Ping value raw mode: <" + myME310.buffer_cstr_raw() + ">");
  }
  else
  {
    Serial.println((String) "Error: " + myME310.return_string(rc));
  }
  Serial.println("The application has ended...");
  exit(0);
}
