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
    In this example sketch, it is shown to use M2M management, using ME310 library.\n
    Check directory, create a new directory, add file in directory, get list of directory, delete files and directory functions are shown.\n


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

char dir[] = "/mod/dir1";
char dataStr[] = "Some data to write\nAnother row";

void setup() {
   
  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.begin(115200);
  myME310.begin(115200);

  delay(1000);
  Serial.println("Telit Test AT M2M command");
  myME310.powerOn();
  Serial.println("ME310 is ON");

  Serial.println("AT Command");
 
  myME310.report_mobile_equipment_error(2); //issue command AT+CMEE=2 and wait for answer or timeout
}

void loop() {
  Serial.print("Check directory: ");
  myME310.read_m2m_chdir();                   //issue command AT#M2MCHDIR in read mode
  Serial.println(myME310.buffer_cstr(1));
  Serial.print("Create new dir: ");
  rc = myME310.m2m_mkdir(dir);                //issue command AT#M2MMKDIR=namedir and wait for answer or timeout
  Serial.println(myME310.return_string(rc));  //returns a string with return_t codes
  if(rc == ME310::RETURN_VALID)
  {
    Serial.println("List directory content: ");
    myME310.m2m_list();                         //issue command AT#M2MLIST and wait for answer or timeout
    Serial.println(myME310.buffer_cstr_raw());  //print modem answer in raw mode

    Serial.println("-------------------------");
    Serial.print("Write file: ");
    rc = myME310.m2m_write_file("/mod/dir1/example.bin", strlen(dataStr), 0, dataStr); //issue command AT#M2MWRITE=path/filename,size,binToMode,data and wait for answer or timeout
    Serial.println(myME310.return_string(rc));
    if(rc == ME310::RETURN_VALID)
    {
      Serial.println("List directory content after write file: ");
      rc = myME310.m2m_list(dir);
      if(rc == ME310::RETURN_VALID)
      {
        Serial.println(myME310.buffer_cstr_raw());
      }
      else
      {
        Serial.println(myME310.return_string(rc));
      }
      Serial.println("-------------------------");
      Serial.print("Read: ");
      rc = myME310.m2m_read("/mod/dir1/example.bin"); //issue command AT#M2MREAD=path/filename and wait for answer or timeout
      Serial.println(myME310.return_string(rc));
      if(rc == ME310::RETURN_VALID)
      {
        Serial.print("Payload raw: ");
        Serial.println((String)"<"+myME310.buffer_cstr_raw()+">");
        Serial.print("Payload not raw: ");
        Serial.println((String)"<"+myME310.buffer_cstr()+">");
      }
      else
      {
        Serial.println(myME310.return_string(rc));
      }
      Serial.println("-------------------------");

      Serial.print("Delete: ");
      rc = myME310.m2m_delete("/mod/dir1/example.bin");  //issue command AT#M2MDEL=path/filename
      Serial.println(myME310.return_string(rc));
      if(rc == ME310::RETURN_VALID)
      {
        Serial.println("List directory content after delete file: ");
        myME310.m2m_list();
        Serial.println(myME310.buffer_cstr_raw());
      }
      else
      {
        Serial.println(myME310.return_string(rc));
      }
      Serial.println("-------------------------");
      Serial.print("Remove dir: ");
      rc = myME310.m2m_rmdir(dir);                    //issue command AT#M2MRMDIR=directoryName
      Serial.println(myME310.return_string(rc));
      if(rc == ME310::RETURN_VALID)
      {
        Serial.println("List directory content after delete dir: ");
        myME310.m2m_list();
        Serial.println(myME310.buffer_cstr_raw());
      }
      else
      {
        Serial.println(myME310.return_string(rc));
      }
    }
  }
  Serial.println("The application has ended...");
  exit(0);
}
