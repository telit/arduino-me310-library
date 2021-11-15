/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    ME310.cpp
    string.h
    stdio.h

  @brief
    Sample test of the use of Generic Command method via ME310 library

  @details
    In this example sketch, the use of method offered by the ME310 library for using AT commands is shown.\n
    Through this method, it is possible to send a generic AT command or a list of AT commands.\n


  @version
    1.0.0

  @note

  @author
    Cristina Desogus

  @date
    08/11/2021
 */
#include <Arduino.h>
#include <ME310.h>

#ifndef ARDUINO_TELIT_SAMD_CHARLIE
#define ON_OFF 6 /*Select the GPIO to control ON_OFF*/
#endif

using namespace me310;

ME310 myME310;
ME310::return_t myRc;
void setup() {
  int i = 0;
  const char OKanswer[] = "OK";
  Serial.begin(115200);
  myME310.begin(115200);
  delay(1000);
  Serial.println("Generic AT Command test");
  delay(1000);
  myME310.powerOn();
  Serial.println("ME310 is ON");

  /*
    ATCommand is the command string which contains the AT commands to send.
    The single command must be separated from the ;.
    The "AT" string must not be repeated.
	For example: "AT#GPIO=1,1,1;#GPIO=2,1,1;#GPIO=3,1,1"
  */
  const char ATCommand[] = "AT#SWPKGV;#CGMM";
  Serial.println((String)"Send: " + ATCommand);
  myRc = myME310.send_command(ATCommand, OKanswer);     //issue a list of command indicated in the string and wait for anwser or timeout
  Serial.println((String) "Result value: " + myME310.return_string(myRc));  //print the anwser value
  while(myME310.buffer_cstr(i) != NULL)
  {
    Serial.println(myME310.buffer_cstr(i));  //print line of modem answer
    i++;
  }

  const char readCommand[] = "at+cops?";
  Serial.println((String)"Send: " + readCommand);
  myRc = myME310.send_command(readCommand, OKanswer);   //issue a command indicated in the string and wait for anwser or timeout
  Serial.println((String) "Result value: " + myME310.return_string(myRc));
  Serial.println(myME310.buffer_cstr(1));

  const char testCommand[] = "at+cmee=?";
  Serial.println((String)"Send: " + testCommand);
  myRc = myME310.send_command(testCommand, OKanswer);
  Serial.println((String) "Result value: " + myME310.return_string(myRc));
  Serial.println(myME310.buffer_cstr(1));
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("The application has ended...");
  exit(0);
}
