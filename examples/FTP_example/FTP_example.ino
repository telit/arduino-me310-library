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
    In this example sketch, it is shown how to use FTP management, using ME310 library.\n
	NOTE:\n
	For correct operation it is necessary to set the correct APN, username and password.


  @version
    1.0.0

  @note

  @author
    Cristina Desogus

  @date
    25/02/2021
 */

#include <ME310.h>
#include <string.h>

#define APN "apn"

#define FTP_ADDR_PORT "ftp.telit.com"
#define FTP_USER "Username"
#define FTP_PASS "Password"

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


void setup() {

  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  myME310.begin(115200);

  delay(1000);
  Serial.println("Telit Test AT FTP command");
  myME310.powerOn();
  Serial.println("ME310 ON");

  Serial.println("AT Command");
  myME310.report_mobile_equipment_error(2);           //issue command AT+CMEE=2 and wait for answer or timeout
  rc = myME310.read_enter_pin();                            //issue command AT+cpin? in read mode, check that the SIM is inserted and the module is not waiting for the PIN
  if(strcmp(myME310.buffer_cstr(2), "OK") == 0)        // read response in 2 array position
  {
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
    }
  }
  else
  {
      Serial.println((String)"Error: " + rc + " Error string: " + myME310.buffer_cstr(2));
  }
}

void loop() {

  char data[] = "We are appending some FTP data\nAnother data";

  rc = myME310.ftp_close();
  if(rc == ME310::RETURN_VALID)
  {
    delay(2000);
    rc = myME310.ftp_open(FTP_ADDR_PORT, FTP_USER, FTP_PASS, 0, cID, ME310::TOUT_2MIN); //issue command AT#FTPOPEN=ftp address and port, ftp user and ftp password and wait for answer or timeout
    Serial.println(myME310.buffer_cstr(0));
    Serial.print("Ftp open: ");
    Serial.println(myME310.return_string(rc));
    if(rc == ME310::RETURN_VALID)
    {
      Serial.println("Ftp list in current directory: ");
      myME310.ftp_list(ME310::TOUT_10SEC);
      Serial.println(myME310.buffer_cstr_raw());

      Serial.print("Ftp Change Working Directory: ");
      rc = myME310.ftp_change_working_directory("/CristinaDe", ME310::TOUT_10SEC); //issue command AT#FTPCWD=path directory and wait for answer or timeout
      Serial.println(myME310.return_string(rc));    //returns a string with return_t codes
      if(rc == ME310::RETURN_VALID)
      {
        Serial.print("Ftp working directory: ");
        myME310.ftp_print_working_directory(ME310::TOUT_10SEC); //issue command AT#FTPPWD and wait for answer or timeout
        Serial.println(myME310.buffer_cstr(1));

        Serial.println("Ftp list in current directory: ");
        myME310.ftp_list(ME310::TOUT_10SEC);
        Serial.println("-------------");
        Serial.println("Buffer cstr raw");
        Serial.println(myME310.buffer_cstr_raw());
        rc = myME310.ftp_put("test.txt",1, ME310::TOUT_10SEC); //issue command AT#FTPPUT=filename,connMode and wait for answer or timeout
        Serial.println((String)"Ftp put result: " +myME310.return_string(rc));
        if(rc == ME310::RETURN_VALID)
        {
          rc = myME310.ftp_append_extended((int)sizeof(data), data, 1);   //issue command AT#FTPAPPEXT=dataSize, data and wait for answer or timeout
          Serial.println((String)"Ftp append extended result: " +myME310.return_string(rc));
          if(rc == ME310::RETURN_VALID)
          {
            delay(3000);
            myME310.ftp_get_file_size("test.txt", ME310::TOUT_10SEC);
            Serial.print("File size: ");
            Serial.println(myME310.buffer_cstr(0));
            Serial.println(myME310.buffer_cstr(1));
            rc = myME310.ftp_get_command_mode("test.txt",0,  ME310::TOUT_20SEC); //issue command AT#FTPGETPKT=file name and wait for answer or timeout
            Serial.println((String) "Ftp get in command mode result: " + myME310.return_string(rc));
            if(rc == ME310::RETURN_VALID)
            {
              rc = myME310.ftp_receive_data_command_mode((int)sizeof(data),  ME310::TOUT_10SEC); //issue command AT#FTPRECV=block size and wait for answer or timeout
              if(rc == ME310::RETURN_VALID)
              {
                Serial.print("Payload raw <");
                Serial.print(myME310.buffer_cstr_raw()); //print modem answer in raw mode
                Serial.println(">");
              }
              else
              {
                Serial.println((String)"ERROR: " + myME310.return_string(rc));
              }
            }
            else
            {
              Serial.println((String)"ERROR: " + myME310.return_string(rc));
            }
          }
          rc = myME310.ftp_delete("test.txt",  ME310::TOUT_30SEC); //issue command AT#FTPDELE=file name and wait for answer or timeout
          if(rc == ME310::RETURN_VALID)
          {
            Serial.println((String)"Ftp Delete result: " +myME310.return_string(rc));
          }
          else
          {
            Serial.println((String)"ERROR: " + myME310.return_string(rc));
          }
        }
      }
      else
      {
        Serial.println((String)"ERROR: " + myME310.return_string(rc));
      }
    }
  }
  else
  {
    Serial.println((String)"ERROR: " + myME310.return_string(rc));
  }
  rc = myME310.ftp_close();
  Serial.println("The application has ended...");
  exit(0);
}
