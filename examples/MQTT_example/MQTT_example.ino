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
    In this example sketch, it is shown how to use MQTT management, using ME310 library.\n
    MQTT enable, MQTT configuration, MQTT connect, MQTT topic subscribe, MQTT publish, MQTT read
    and MQTT disconnect methods are used and responses are printed.\n
	NOTE:\n
	For the sketch to work properly, you need to set the correct APN, client_id, client_username, client_password.


  @version
    1.1.0

  @note

  @author
    Cristina Desogus

  @date
    15/06/2021
 */


#include <ME310.h>
#include <string.h>

#define APN "APN"
#define HOSTNAME "api-dev.devicewise.com"
#define PORT 1883

#define CLIENT_ID "CLIENTID"
#define CLIENT_USERNAME "CLIENTUSERNAME"
#define CLIENT_PASSWORD "PASSWORD"

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

int cID = 1;          //PDP Context Identifier
char ipProt[]= "IP";  //Packet Data Protocol type

int count = 0;
bool isConnect = false;

void setup() {

  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  myME310.begin(115200);

  delay(1000);

  myME310.powerOn();
  Serial.println("Telit Test AT MQTT command");
  Serial.println("ME310 ON");
  Serial.println("AT Command");
   ////////////////////////////////////
  // Report Mobile Equipment Error CMEE
  // (0 -> disable, 1-> enable numeric
  // values, 2 -> enable verbose mode)
  ///////////////////////////////////
  myME310.report_mobile_equipment_error(2);           //issue command AT+CMEE=2 and wait for answer or timeout
  rc = myME310.read_enter_pin();                        //issue command AT+cpin? in read mode, check that the SIM is inserted and the module is not waiting for the PIN
  char *resp = (char* )myME310.buffer_cstr(2);
  if(resp != NULL)
  {
    if(strcmp(resp, "OK") == 0)        // read response in 2 array position
    {
      Serial.println("Define PDP Context");
      rc = myME310.define_pdp_context(cID, ipProt, APN);   //issue command AT+CGDCONT=cid,PDP_type,APN
      Serial.print(rc);
      if(rc == ME310::RETURN_VALID)
      {
        myME310.read_define_pdp_context();              //issue command AT+CGDCONT=? (read mode)
        Serial.print("pdp context read: ");
        Serial.println(myME310.buffer_cstr(1));              //print second line of modem answer

        Serial.print("gprs network registration status: ");
        rc = myME310.read_gprs_network_registration_status();  //issue command AT+CGREG=? (read mode)
        if(rc == ME310::RETURN_VALID)
        {
          resp = (char*)myME310.buffer_cstr(1);
          Serial.println(resp);
          while(resp != NULL)
          {
            if ((strcmp(resp, "+CGREG: 0,1") != 0) &&  (strcmp(resp, "+CGREG: 0,5") != 0))
            {
                delay(3000);
                rc = myME310.read_gprs_network_registration_status();
                if(rc != ME310::RETURN_VALID)
                {
                  Serial.println("ERROR");
                  break;
                }
                resp = (char*)myME310.buffer_cstr(1);
                Serial.println(resp);
            }
            else
            {
              break;
            }
          }
        }
        ///////////////////////////////////
        // Context Activation
        // AT#SGACT=cid,stat (1 enable)
        ///////////////////////////////////
        Serial.println("Activate context");
        myME310.context_activation(cID, 1);        //issue command AT#SGACT=cid,state and wait for answer or timeout

        ///////////////////////////////////
        // AT#MQEN?
        ///////////////////////////////////
        Serial.println("mqtt enable read");
        rc = myME310.read_mqtt_enable();          //issue command AT#MQEN? and wait for answer or timeout
        Serial.println(myME310.buffer_cstr(1));
        resp = (char*) myME310.buffer_cstr(1);
        if(resp != NULL)
        {
          if ((strcmp(resp, "#MQEN: 1,0")) == 0)
          {
            ///////////////////////////////////
            // AT#MQEN=instance Number, state (1 enable)
            ///////////////////////////////////
            Serial.println("mqtt enable");
            rc = myME310.mqtt_enable(1, 1);       //issue command AT#MQEN=instance_number,state and wait for answer or timeout
            Serial.println(myME310.buffer_cstr(1));
            rc = myME310.read_mqtt_enable();
            Serial.println(myME310.buffer_cstr(1));
          }
        }
        delay(1000);
        ///////////////////////////////////
        // AT#MQCFG=instance Number, hostname, port, cid, sslEN
        ///////////////////////////////////
        Serial.print("mqtt configure:");
        rc = myME310.mqtt_configure(1, HOSTNAME , PORT, cID); //issue command  AT#MQCFG=instance Number, hostname, port, cid, sslEN and wait for answer or timeout
        Serial.println(myME310.buffer_cstr(1));
        delay(1000);
        ///////////////////////////////////
        // AT#MQCONN=instance Number, client_id, username, password
        ///////////////////////////////////
        if(rc == ME310::RETURN_VALID)
        {
          Serial.print("mqtt connect: ");
          rc = myME310.mqtt_connect(1, CLIENT_ID, CLIENT_USERNAME, CLIENT_PASSWORD, ME310::TOUT_1MIN); //issue command  AT#MQCONN=instance Number, client_id, username, password and wait for answer or timeout
          Serial.println(myME310.buffer_cstr(1));
          if(rc == ME310::RETURN_VALID)
          {
            isConnect = true;
            ////////////////////////////////////
            // COMMAND TO SUBSCRIBE
            //
            // AT#MQSUB=instance_number, topic
            /////////////////////////////////////
            Serial.print("MQTT Topic Subscribe: ");
            myME310.mqtt_topic_subscribe(1,"topic");  //issue command AT#MQSUB=instance_number, topic and wait for answer or timeout
            Serial.println(myME310.buffer_cstr(1));

            /////////////////////////////////////
            // COMMAND TO PUBLISH
            //
            // AT#MQPUBS=instance_number, topic, retain, qos, message
            ////////////////////////////////////
            Serial.print("MQTT Publish: ");
            myME310.mqtt_publish(1, "topic", 1, 0, "message");  //issue command AT#MQPUBS=instance_number, topic, retain, qos, message and wait for answer or timeout
            Serial.println(myME310.buffer_cstr(1));

          }
        }
      }
    }
    else
    {
        Serial.println((String)"Error: " + rc + " Error string: " + myME310.buffer_cstr(2));
    }
  }
  else
  {
      Serial.println((String)"Error: " + rc + " Error string: " + myME310.buffer_cstr(2));
  }
}

void loop(){
  if(isConnect)
  {
  /////////////////////////////////////
    // COMMAND TO CHECK IF SOME MESSAGES HAVE ARRIVED
    //
    // AT#MQREAD?
    ////////////////////////////////////
    Serial.print("MQTT read value: ");
    myME310.read_mqtt_read();       //issue command AT#MQREAD? and wait for answer or timeout
    Serial.println(myME310.buffer_cstr(1));
    char *resp = (char* )myME310.buffer_cstr(1);
    if(resp != NULL)
    {
      if ((strcmp(resp, "#MQREAD: 1,1")) == 0)
      {
        /////////////////////////////////////
        // COMMAND TO READ A MESSAGE
        //
        // AT#MQREAD=instance_number, id_message
        ////////////////////////////////////
        Serial.print("MQTT read: ");
        myME310.mqtt_read(1,1);                     //issue command AT#MQREAD=instance_number, id_message and wait for answer or timeout
        Serial.println(myME310.buffer_cstr_raw());  //print response in raw mode
        myME310.mqtt_disconnect(1);   //AT#MQDISC=instance_number
        exit(0);
      }
      else
      {
        count++;
      }
    }
    
    if (count == 5)
    {
        /////////////////////////////////////
        // COMMAND TO DISCONNECT
        //
        // AT#MQDISC=instance_number
        ////////////////////////////////////
        myME310.mqtt_disconnect(1);   //AT#MQDISC=instance_number
        exit(0);
    }
  }
  else
  {
    Serial.println("Connection error");
    exit(0);
  }
}
