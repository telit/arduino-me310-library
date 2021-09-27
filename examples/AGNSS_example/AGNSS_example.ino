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
    In this example sketch, it is shown how to use AGNSS management, using ME310 library.\n
    AGNSS is enable, GNSS configuration, GNSS controller power management, GNSS nmea configuration functions are shown.\n
    GPS positions are acquired and response is printed.


  @version 
    1.0.0
  
  @note

  @author
    Cristina Desogus

  @date
    05/07/2021
 */

#include <ME310.h>
#include <string>  

/*When NMEA_DEBUG is 0 Unsolicited NMEA is disable*/
#define NMEA_DEBUG 0

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

ME310::return_t rc; 
int count = 0;

void setup() {
  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(ON_OFF, LOW);

  Serial.begin(115200);
  myME310.begin(115200);
  delay(1000);
  myME310.powerOn(ON_OFF);
  delay(5000);
  Serial.println("Telit Test AT GNSS command");
  Serial.println("ME310 ON");
  Serial.println("AT Command");
  ME310::return_t rc = myME310.attention();    // issue command and wait for answer or timeout
  Serial.println(myME310.buffer_cstr(0));       // print first line of modem answer
  ////////////////////////////////////
  // Report Mobile Equipment Error CMEE
  // (0 -> disable, 1-> enable numeric
  // values, 2 -> enable verbose mode)
  ///////////////////////////////////
  myME310.report_mobile_equipment_error(2);           //issue command AT+CMEE=2 and wait for answer or timeout

  /////////////////////////////////////
  // Read GNSS configuration
  // $GPSCFG:<startup priority>,<TBF>,<costellation>,<runtime priority>
  // e.g. $GPSCFG: 0,4321,2,0
  /////////////////////////////////////
  Serial.println("Read GNSS configuration");
  myME310.read_gnss_configuration();            //issue command AT$GPSCFG? 
  Serial.println(myME310.buffer_cstr(1));
  /////////////////////////////////////
  // Set GNSS priority
  // AT$GPSCFG=0,0
  // 0 : priority GNSS
  // 1 : priority WWAN
  /////////////////////////////////////
  Serial.println("Set GNSS priority");
  rc = myME310.gnss_configuration(0,0);
  Serial.println(ME310::return_string(rc));      // print return value

  if (rc == ME310::RETURN_VALID)
  {
    myME310.module_reboot();              //issue command AT#REBOOT
    Serial.println(myME310.buffer_cstr(1));
  }

  delay(5000);
  /////////////////////////////////////
  // Set constellations 
  // AT$GPSCFG=2,X
  // 0 : the constellation is selected automatically based on Mobile Country Code (MCC) of camped network
  // 1 : GPS+GLO
  // 2 : GPS+GAL
  // 3 : GPS+BDS
  // 4 : GPS+QZSS
  /////////////////////////////////////
  Serial.println("Set constellations");
  rc = myME310.gnss_configuration(2,1);
  Serial.println(ME310::return_string(rc));
  if (rc == ME310::RETURN_VALID)
  {
    myME310.module_reboot();              //issue command AT#REBOOT
    Serial.println(myME310.buffer_cstr(1));
  }
  delay(5000);
  /////////////////////////////////////
  // Set  priority to GNSS priority in runtime
  // AT$GPSCFG=3,0
  // 0 : priority GNSS
  // 1 : priority WWAN
  /////////////////////////////////////
  Serial.println("Set  priority to GNSS priority in runtime");
  rc = myME310.gnss_configuration(3,0);
  Serial.println(ME310::return_string(rc));
  if (rc == ME310::RETURN_VALID)
  {
    myME310.module_reboot();              //issue command AT#REBOOT
    Serial.println(myME310.buffer_cstr(1));
  }
  delay(5000);


  /////////////////////////////////////
  // Set on/off GNSS controller
  // AT$GPSP=<status>
  // 0 : GNSS controller is powered down
  // 1 : GNSS controller is powered up
  /////////////////////////////////////
  Serial.println("Set on/off GNSS controller");
  rc = myME310.gnss_controller_power_management(1);
  Serial.println(ME310::return_string(rc));
  if (rc == ME310::RETURN_VALID)
  {
    #if NMEA_DEBUG
    /////////////////////////////////////
    // Set GNGSA, GLGSV and GNRMC as available sentence in the unsolicited NMEA sentences.
    // AT$GPSNMUNEX=0,1,1,0,0,0,0,0,0,0,0,1,0
    /////////////////////////////////////
    rc = myME310.gnss_nmea_extended_data_configuration(0,1,1,0,0,0,0,0,0,0,0,1,0); 
    if(rc ==  ME310::RETURN_VALID)
    {
      /////////////////////////////////////
      //  Activate unsolicited NMEA sentences flow in the AT port and GPGGA,GPRMC, GPGSA and GPGSV sentences.
      //  AT$GPSNMUN=2,1,0,1,1,1,0
      /////////////////////////////////////
      rc = myME310.gnss_nmea_data_configuration(2,1,0,1,1,1,0);
      int i = 0;
      while(strcmp(myME310.buffer_cstr(i), "OK") != 0)
      {
        Serial.println(myME310.buffer_cstr(i));
        i++;
      }
    }
    #endif
  }

  /////////////////////////////////////
  //  Set the real-time clock of the module.
  //  AT+CCLK="05/07/21,12:40:00+00"
  /////////////////////////////////////
  Serial.println("Set the real-time clock of the module.");
  rc = myME310.clock_management("05/07/21,12:40:00+00");
  Serial.println(ME310::return_string(rc));

  /////////////////////////////////////
  //  Set the AGNSS providers enable or disable.
  //  AT$AGNSS=provider,status
  //  <status>:0 to disable
  //           1 to enable
  ///////////////////////////////////// 
  Serial.println("Set the AGNSS providers enable.");
  myME310.gnss_set_agnss_enable(0,1);

  myME310.module_reboot();
  delay(10000);

  /////////////////////////////////////
  //  Read command returns the requested and the active status for each agnss provider.
  //  $AGNSS: <provider>,<active>,<requested>
  //  Request value is 0 to disable or 1 to enable
  /////////////////////////////////////
  myME310.read_gnss_set_agnss_enable();
  Serial.println(myME310.buffer_cstr(1));
}

void loop() {
  delay(1000);
  /////////////////////////////////////
  //  Get Acquired Position
  //  AT$GPSACP
  /////////////////////////////////////
  
  rc = myME310.gps_get_acquired_position();

  /*When the position is fixed, the led blinks*/
  if (rc == ME310::RETURN_VALID)
  {
    Serial.println(myME310.buffer_cstr(1));
    std::string tmp_pos;
    tmp_pos = myME310.buffer_cstr(1);
    std::size_t len_pos = tmp_pos.find(":");
    if(len_pos != std::string::npos)
    {
        std::size_t len_pos2 = tmp_pos.find(",");
        char valid_pos[64];
        if(len_pos2 != std::string::npos)
        {
          int len = tmp_pos.copy(valid_pos, ((len_pos2-1)-len_pos), len_pos+1);
          if(len > 2)
          {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(6000);
            digitalWrite(LED_BUILTIN, LOW);
            delay(1000);
          }
        }
    }
  }
  delay(5000);
  if(count > 20)
  {
    #if NMEA_DEBUG
      //////////////////////////////////
      //Stop NMEA Flow and Stop GNSS Session
      //////////////////////////////////
      Serial.println("Stop NMEA Flow and GNSS Session");
      /////////////////////////////////////
      // De-activate unsolicited NMEA sentences flow.
      // AT$GPSNMUN=0
      /////////////////////////////////////
      rc = myME310.gnss_nmea_extended_data_configuration(0);
      if(rc == ME310::RETURN_VALID)
      {
      Serial.println("De-activate unsolicited NMEA sentences flow");
      }
    #endif
    /////////////////////////////////////
    // GNSS controller is powered down
    // AT$GPSP=0
    /////////////////////////////////////
    Serial.println("Stop GNSS Session");
    myME310.gnss_controller_power_management(0);
    delay(5000);
    exit(0);
  }
  count++;
}