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
    In this sketch, the operation of the get object functions in LWM2M objects is shown.

	NOTE:\n


  @version
    1.0.0

  @note

  @author
    Cristina Desogus

  @date
    24/01/2022
 */


#include <Arduino.h>

#include <ME310.h>
#include <string>


#define APN "web.omnitel.it"

#define ON_OFF 6 /*Select the GPIO to control ON_OFF*/


using namespace me310;

ME310 myME310;
ME310::return_t myRc;
char ipProt[] = "IP";
const char* pin = 0;
bool setupValid = true;
int CID = 1;
int count = 0;


void setup() {

  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(100);
  Serial.begin(115200);
  myME310.begin(115200);
  delay(1000);
  myME310.powerOn(ON_OFF);
  delay(2000);

  Serial.println("Start LWM2M example");
  myRc = myME310.report_mobile_equipment_error(2);  //issue command AT+CMEE=2 and wait for answer or timeout
  if(checkSIM())
  {
    if(PDPContext())
    {
      if(registrationStatus())
      {
        if(LWM2MEnable())
        {
          Serial.println("Configuration completed successfully");
        }
        else
        {
          setupValid = false;
        }
      }
      else
      {
        setupValid = false;
      }
    }
    else
    {
      setupValid = false;
    }
  }
  else
  {
    setupValid = false;
  }
  // put your setup code here, to run once:

}

void loop() {

  if(!setupValid)
  {
    Serial.println("Configuration ERROR");
    exit(0);
  }
  else
  {
    Serial.println("Get Object 6");
    myRc = myME310.LWM2M_get_object(0,6);   //issue command AT#LWM2MOBJGET=agentInstanceID,objectID
    if(myRc != ME310::RETURN_VALID)
    {
      exit(0);
    }

    Serial.print("<");
    Serial.print(myME310.buffer_cstr_raw());
    Serial.println(">");

    Serial.println("Get Object 6 instance 0");
    myRc = myME310.LWM2M_get_object_instance(0,6,0);   //issue command AT#LWM2MOBJGET=agentInstanceID,objectID,objectInstanceID
    if(myRc != ME310::RETURN_VALID)
    {
      exit(0);
    }

    Serial.print("<");
    Serial.print(myME310.buffer_cstr_raw());
    Serial.println(">");

    Serial.println("Get Object 6 instance 0 resource 1");
    myRc = myME310.LWM2M_get_object_resource(0,6,0,1);   //issue command AT#LWM2MOBJGET=agentInstanceID,objectID,objectInstanceID,resourceID
    if(myRc != ME310::RETURN_VALID)
    {
      exit(0);
    }

    Serial.print("<");
    Serial.print(myME310.buffer_cstr_raw());
    Serial.println(">");
  }
  Serial.println("End Application.");
  exit(0);
}

//! \brief Checks SIM
/*! \details
This function checks for the presence of the SIM and its management.
 *
 * \return returns true if the SIM and its management is done correctly, otherwise false.
 */
bool checkSIM()
{
  bool rc = false;
  Serial.println("Check SIM");
  myRc = myME310.read_enter_pin();    //issue command AT+cpin? in read mode, check that the SIM is inserted and the module is not waiting for the PIN
  String result = myME310.buffer_cstr(1);
  if(result.endsWith("READY"))
  {
      Serial.println("SIM is READY");
      delay(5000);
      rc = true;
  }
  else if (result.endsWith("SIM PIN"))
  {
    if (pin != NULL)
    {
      myME310.enter_pin(pin);     //issue command AT+cpin=PIN
      result = myME310.buffer_cstr(1);
      if(result.endsWith("READY"))
      {
          Serial.println("SIM is READY");
          rc = true;
      }
      else
      {
        rc = false;
      }
    }
  }
  else
  {
    rc = false;
  }
  return rc;
}

//! \brief PDP Context management.
/*! \details
This function deals with the management of the PDP Context.
 *
 * \return returns true if the management is done correctly, otherwise false.
 */
bool PDPContext()
{
  bool rc = false;
  myRc = myME310.define_pdp_context(CID, ipProt, APN);  //issue command AT+CGDCONT=cid,PDP_type,APN
  if(myRc == ME310::RETURN_VALID)
  {
    rc = true;
  }
  else
  {
    rc = false;
  }
  return rc;
}

//! \brief Checks GPRS network registration status.
/*! \details
This function takes care of checking the GPRS network registration status.
 *
 * \return returns true if the checking is done correctly, otherwise false.
 */
bool registrationStatus()
{
  bool rc = true;
  Serial.println("GPRS network registration status");
  myRc = myME310.read_gprs_network_registration_status();  //issue command AT+CGREG=? (read mode)
  if(myRc == ME310::RETURN_VALID)
  {
    char *resp = (char*) myME310.buffer_cstr(1);
    while(resp != NULL)
    {
      if ((strcmp(resp, "+CGREG: 0,1") != 0) && (strcmp(resp, "+CGREG: 0,5") != 0))
      {
        delay(3000);
        myRc = myME310.read_gprs_network_registration_status();
        if(myRc != ME310::RETURN_VALID)
        {
          Serial.print("Read GPRS network registration status: ");
          Serial.println(myME310.return_string(myRc));
          rc = false;
          break;
        }
        Serial.println(myME310.buffer_cstr(1));
        resp = (char* )myME310.buffer_cstr(1);
      }
      else
      {
        break;
      }
    }
  }
  else
  {
    rc = false;
  }
  return rc;
}

//! \brief Enable LWM2M
/*! \details
This function is responsible for managing the enable of the LWM2M, checking the existence of objects and creating a new instance for objects 3 and 3313.
 *
 * \return returns true if the management is done correctly, otherwise false.
 */
bool LWM2MEnable()
{
  bool ret = false;
  Serial.println("LWM2MEnable");
  myRc = myME310.read_enableLWM2M();  //issue command AT#LWM2MENA?, in read mode.
  String resp = (char*)myME310.buffer_cstr(1);
  Serial.println(resp.c_str());
  if(resp.endsWith("\"ACTIVE\""))
  {
    ret = true;
  }
  else
  {
    myRc = myME310.enableLWM2M(1,1);
    delay(1000);
    myRc = myME310.read_enableLWM2M();  //issue command AT#LWM2MENA?, in read mode.
    String resp = (char*)myME310.buffer_cstr(1);
    Serial.println(resp.c_str());
    if(resp.endsWith("\"ACTIVE\""))
    {
      ret = true;
    }
    else
    {
      ret = false;
    }
  }
  return ret;
}