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
    In this example sketch, the use of methods offered by the ME310 library for using AT commands is shown.

  @version 
    1.0.0
  
  @note

  @author
    Cristina Desogus

  @date
    02/07/2021
 */

#include <ME310.h>
#include <string>  

/*When NMEA_DEBUG is 0 Unsolicited NMEA is disable*/
#define NMEA_DEBUG 0

using namespace me310;

ME310 myME310;

ME310::return_t rc; 
int count = 0;

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
  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(ON_OFF, LOW);

  Serial.begin(115200);
  myME310.begin(115200);
  delay(1000);
  turnOnModule();
  delay(5000);
  Serial.println("Telit Test AT GNSS command");    
  Serial.println("ME310 ON");  
  Serial.println("AT Command");
  ME310::return_t rc = myME310.attention();    // issue command and wait for answer or timeout
  Serial.println(myME310.buffer_cstr(0));       // print first line of modem answer
  Serial.print(ME310::return_string(rc));      // print return value
  ////////////////////////////////////
  // Report Mobile Equipment Error CMEE
  // (0 -> disable, 1-> enable numeric
  // values, 2 -> enable verbose mode)
  ///////////////////////////////////
  myME310.report_mobile_equipment_error(2);           //issue command AT+CMEE=2 and wait for answer or timeout

  myME310.read_gnss_configuration();            //issue command AT$GPSCFG? 
  Serial.println(myME310.buffer_cstr(1));

  /////////////////////////////////////
  // Set GNSS priority
  // AT$GPSCFG=0,0
  // 0 : priority GNSS
  // 1 : priority WWAN
  /////////////////////////////////////
  rc = myME310.gnss_configuration(0,0); 
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
  rc = myME310.gnss_configuration(2,1);
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

  rc = myME310.gnss_configuration(3,0);
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
    rc = myME310.gnss_controller_power_management(1);
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

