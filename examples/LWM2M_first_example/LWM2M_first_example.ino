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
    In this sketch, the operation of the read and set functions in LWM2M objects is shown.\n
    The resources Model Number (1), Serial Number (2), Firmware Version (3), of the Device object (3) are read.\n
    In object 3313 (Accelerometer), set in resources x (5702), y (5703), z (5704).\n

	NOTE:\n
	  For the sketch to work correctly, it is necessary to load the XML of the object 3313 on the board.\n

  @version
    1.0.0

  @note

  @author
    Cristina Desogus

  @date
    14/01/2022
 */


#include <Arduino.h>
#include <telit_bma400.h>

#include <ME310.h>
#include <string>


#define APN "APN"


#define LWM2M_EXIST_DEVICE "AT#LWM2MEXIST=0,3,0"
#define LWM2M_NEWINST_DEVICE "AT#LWM2MNEWINST=0,3,0"

#define LWM2M_EXIST_ACCELEROMETER "AT#LWM2MEXIST=0,3313,0"
#define LWM2M_NEWINST_ACCELEROMETER "AT#LWM2MNEWINST=0,3313,0"

#define OK_ANSWER "OK"

#define ON_OFF 6 /*Select the GPIO to control ON_OFF*/

/* Earth's gravity in m/s^2 */
#define GRAVITY_EARTH     (9.80665f)

/* 39.0625us per tick */
#define SENSOR_TICK_TO_S  (0.0000390625f)

using namespace me310;

ME310 myME310;
ME310::return_t myRc;
char ipProt[] = "IP";
const char* pin = 0;
bool setupValid = true;
int CID = 1;
int count = 0;

static float lsb_to_ms2(int16_t accel_data, uint8_t g_range, uint8_t bit_width);

struct bma400_dev bma;
struct bma400_sensor_conf conf;
struct bma400_sensor_data data;
struct bma400_int_enable int_en;
int8_t rslt;

float x, y, z;
uint16_t int_status = 0;

static float lsb_to_ms2(int16_t accel_data, uint8_t g_range, uint8_t bit_width)
{
    float accel_ms2;
    int16_t half_scale;

    half_scale = 1 << (bit_width - 1);
    accel_ms2 = (GRAVITY_EARTH * accel_data * g_range) / half_scale;

    return accel_ms2;
}

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
          /* Interface reference is given as a parameter
          *         For I2C : BMA400_I2C_INTF
          */
          rslt = bma400_interface_init(&bma, BMA400_I2C_INTF);

          bma400_check_rslt("bma400_interface_init", rslt);

          rslt = bma400_soft_reset(&bma);
          bma400_check_rslt("bma400_soft_reset", rslt);

          rslt = bma400_init(&bma);
          bma400_check_rslt("bma400_init", rslt);

          /* Select the type of configuration to be modified */
          conf.type = BMA400_ACCEL;

          /* Get the accelerometer configurations which are set in the sensor */
          rslt = bma400_get_sensor_conf(&conf, 1, &bma);
          bma400_check_rslt("bma400_get_sensor_conf", rslt);

          /* Modify the desired configurations as per macros
          * available in bma400_defs.h file */
          conf.param.accel.odr = BMA400_ODR_100HZ;
          conf.param.accel.range = BMA400_RANGE_2G;
          conf.param.accel.data_src = BMA400_DATA_SRC_ACCEL_FILT_1;

          /* Set the desired configurations to the sensor */
          rslt = bma400_set_sensor_conf(&conf, 1, &bma);
          bma400_check_rslt("bma400_set_sensor_conf", rslt);

          rslt = bma400_set_power_mode(BMA400_MODE_NORMAL, &bma);
          bma400_check_rslt("bma400_set_power_mode", rslt);

          int_en.type = BMA400_DRDY_INT_EN;
          int_en.conf = BMA400_ENABLE;

          rslt = bma400_enable_interrupt(&int_en, 1, &bma);
          bma400_check_rslt("bma400_enable_interrupt", rslt);
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
    myRc = myME310.readResourceString(0,3,0,1,0);
    if(myRc == ME310::RETURN_VALID)
    {
      Serial.println(myME310.buffer_cstr(1));
    }
    myRc = myME310.readResourceString(0,3,0,2,0);
    if(myRc == ME310::RETURN_VALID)
    {
      Serial.println(myME310.buffer_cstr(1));
    }
    myRc = myME310.readResourceString(0,3,0,3,0);
    if(myRc == ME310::RETURN_VALID)
    {
      Serial.println(myME310.buffer_cstr(1));
    }

    Serial.print("\nGet accel data - BMA400_DATA_SENSOR_TIME\n");

    while ((rslt == BMA400_OK))
    {
      rslt = bma400_get_interrupt_status(&int_status, &bma);
      if (count < 20)
      {
        if (int_status & BMA400_ASSERTED_DRDY_INT)
        {
          rslt = bma400_get_accel_data(BMA400_DATA_SENSOR_TIME, &data, &bma);
          /* 12-bit accelerometer at range 2G */
          x = lsb_to_ms2(data.x, 2, 12);
          y = lsb_to_ms2(data.y, 2, 12);
          z = lsb_to_ms2(data.z, 2, 12);
          Serial.println("");
          Serial.println("Accelation values by bma400_get_accel_data function: ");
          Serial.println((String)"X-> " + x + ", Y-> "+ y+ ", Z-> " +z);

          myME310.setResourceFloat(3313,0,5702,0,x);
          myME310.setResourceFloat(3313,0,5703,0,y);
          myME310.setResourceFloat(3313,0,5704,0,z);
          count++;
          delay(30000);
        }
      }
      else
      {
        Serial.println("The application has ended... ");
        exit(0);
      }
    }
  }
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
    myRc = myME310.enableLWM2M(1, 1, ME310::TOUT_45SEC);    //issue command AT#LWM2MENA=enable/disable, PDP context identifier
    if (myRc == ME310::RETURN_VALID)
    {
      delay(10000);
      Serial.println("Registered on server DM");
      if(!checkExistAgent(LWM2M_EXIST_DEVICE))
      {
        myRc = myME310.send_command(LWM2M_NEWINST_DEVICE);  //issue command AT#LWM2MNEWINST=agentInstance, objectID, objectInstanceID
        Serial.println(myME310.return_string(myRc));
        if(myRc != ME310::RETURN_VALID)
        {
          ret = false;
        }
        else
        {
          ret = true;
        }
      }
      if(!checkExistAgent(LWM2M_EXIST_ACCELEROMETER))
      {
        myRc = myME310.send_command(LWM2M_NEWINST_ACCELEROMETER);
        Serial.println(myME310.return_string(myRc));
        if(myRc != ME310::RETURN_VALID)
        {
          ret = false;
        }
        else
        {
          ret = true;
        }
      }
    }
    else
    {
      Serial.println("Enabling LWM2M returned error");
      ret = false;
    }
  }
  return ret;
}

//! \brief Checks exist agent.
/*! \details
This function checks whether or not a particular agent exists.
 *
 * \param LWM2MEXISTCommand the command string to send.
 * \return returns true if the answer is positive, otherwise false.
 */
bool checkExistAgent(String LWM2MEXISTCommand)
{
  bool ret = false;
  const char *answer = OK_ANSWER;
  myRc = myME310.send_command(LWM2MEXISTCommand.c_str(), answer); //issue command AT#LWM2MEXIST=agentInstance, objectNumber, objecttInstanceNumber.
  String resp = myME310.buffer_cstr(1);
  if(resp.endsWith("OK"))
  {
    ret = true;
  }
  else
  {
    ret = false;
  }
  return ret;
}