
/*MIT License

  Copyright (c) 2021 Telit

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

/* Sketch that uses the functionality of Telit Charlie board.
  This application allows to detect a freefall and to send an email that warns about it with the GPS coordinates.
  To detect the freefall, the registers of the accelerometer BMA400 by Bosch Sensortec were written following
  the document "How to generate freefall interrupt using BMA400" by Bosch Sensortec.


  Sketch for 2G->AT+WS46=12

  author: Cristina Ceron
*/


#include <ME310.h>
#include <BMA400.h>
using namespace me310;
float x = 0, y = 0, z = 0;
const byte INTERRUPT = ACC_INT_1;
bool isr = false;
bool radius = false;
const char *resp;

#ifndef ARDUINO_TELIT_SAMD_CHARLIE
#define ON_OFF 6 /*Select the GPIO to control ON_OFF*/
#endif
/*
 * If a Telit-Board Charlie is not in use, the ME310 class needs the Uart Serial instance in the constructor, that will be used to communicate with the modem.\n
 * Please refer to your board configuration in variant.h file.
 * Example:
 * Uart Serial1(&sercom4, PIN_MODULE_RX, PIN_MODULE_TX, PAD_MODULE_RX, PAD_MODULE_TX, PIN_MODULE_RTS, PIN_MODULE_CTS);
 * ME310 myME310 (Serial1);
 */
ME310 myME310;


void setup() {
  char pin[] = "XXXX";

  ME310::return_t rc;
  pinMode(ON_OFF, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  myME310.begin(115200);

  delay(1000);
  myME310.powerOn();
  delay(5000);

  // issue command at#reboot
  myME310.module_reboot();

  delay(10000);
  Serial.println("Telit Test AT LWM2M accelerometer");
  myME310.powerOn();
  Serial.println("ME310 ON");

  //issue command AT+CMEE=2 and wait for answer or timeout
  myME310.report_mobile_equipment_error(2);


  //issue command AT+CPIN? in read mode, check that the SIM is inserted and the module is not waiting for the PIN
  myME310.read_enter_pin();

  // read response in 1 array position
  char *resp = (char*)myME310.buffer_cstr(1);
  if(resp != NULL)
  {
    if (strcmp(resp, "+CPIN: SIM PIN") == 0)
    {
      Serial.println("Insert SIM PIN");

      //issue command AT+CPIN=pin
      rc = myME310.enter_pin(pin);
      if (rc == ME310::RETURN_VALID)
      {
        Serial.println("PIN inserted");
        Serial.println("Select wireless tecnology ");

        //issue command AT+WS46=12(2G)
        rc = myME310.select_wireless_network(12);
        if (rc == ME310::RETURN_VALID)
        {
          //issue command AT+CREG? to check the network status
          Serial.println("Network status");
          rc = myME310.read_network_registration_status();
          Serial.println(myME310.buffer_cstr(1));
          if (rc == ME310::RETURN_VALID)
          {
            //if +CREG!=0,1 and +CREG!=0,5, wait 3 second than retry the reading
            resp = (char*)myME310.buffer_cstr(1);

            while(resp != NULL)
            {
                if ((strcmp(resp, "+CREG: 0,1") != 0) &&  (strcmp(resp, "+CREG: 0,5") != 0))
                {
                  delay(3000);
                  rc = myME310.read_network_registration_status();
				  if(rc != ME310::RETURN_VALID)
				  {
					Serial.println("ERROR");
					Serial.println(myME310.return_string(rc));
					break;
				  }
                  Serial.println(myME310.buffer_cstr(1));
                  resp = (char*)myME310.buffer_cstr(1);
                }
                else
                {
                  break;
                }
            }
          }
        }
      }
    }
    else if (strcmp(resp, "+CPIN: READY") == 0)
    {
      Serial.println("Select wireless tecnology ");
      rc = myME310.select_wireless_network(12);  //issue command AT+WS46=12(2G)
      if (rc == ME310::RETURN_VALID)
      {
        //issue command AT+CREG? to check the network status
        Serial.println("Network status");
        rc = myME310.read_network_registration_status();
        Serial.println(myME310.buffer_cstr(1));
        if (rc == ME310::RETURN_VALID)
        {
          //if +CREG!=0,1 and +CREG!=0,5, wait 3 second than retry the reading
          resp = (char*)myME310.buffer_cstr(1);
          while(resp != NULL)
          {
            if ((strcmp(resp, "+CREG: 0,1") != 0) &&  (strcmp(resp, "+CREG: 0,5") != 0))
            {
              delay(3000);
              rc = myME310.read_network_registration_status();
              if(rc != ME310::RETURN_VALID)
              {
                Serial.println("ERROR");
                Serial.println(myME310.return_string(rc));
                break;
              }
              Serial.println(myME310.buffer_cstr(1));
              resp = (char*)myME310.buffer_cstr(1);
            }
            else
            {
              break;
            }
          }
        }
      }
    }
  }


  /*Registers setup for detecting freefall. Some register can be fine-tuned*/
  Wire.begin();
  while (!Serial);
  Serial.println("BMA400 Raw Data");

  while (1)
  {
    if (bma400.isConnection())
    {
      bma400.initialize();
      Serial.println("BMA400 is connected");
      break;
    }
    else
    {
      Serial.println("BMA400 is not connected");
    }
    delay(2000);
  }
  uint8_t i = 0;
  uint8_t *buf = (byte*)malloc(0xFF);

  uint8_t rw = 1;
  uint8_t reg = 0x3F;
  uint8_t buf1[] = {0xF0, 0x01, 0x3F, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint16_t len = sizeof(buf1);

  bma400.readwrite(reg, buf1, len, rw);
  rw = 0;
  reg = 0x00;
  len = 0x50;
  bma400.readwrite(reg, buf, len, rw);
  i = 0;
  Serial.println("\nDevice Initialization");
  Serial.print("Buffer LEN=");
  Serial.print(len, DEC);
  Serial.print("\n");
  if(buf != NULL)
  {
    while (i < len)
    {
      Serial.print("0x");
      Serial.print(reg++, HEX);
      Serial.print("\t0x");
      Serial.print(buf[i++], HEX);
      Serial.print("\n");
    }
  }

  pinMode (INTERRUPT, INPUT);
  digitalWrite (INTERRUPT, LOW);  // internal pull-down resistor
  attachInterrupt(digitalPinToInterrupt(INTERRUPT), freefall, RISING);
}

//Subroutine to manage the interrupt:
//the variable isr is set true
void freefall() {
  Serial.print("\n***********\nFreefall Occurred\n*******\n");
  isr = true;

}

/*Main function that manages the Telit module through AT commands sent by the microprocessor.
  In this function if the isr variable is false the freefall not occurred, else the freefall occurred.
  In the second case, the function does the following steps:
  -enables the LWM2M client through AT#LWM2MENA=1,x, where x is the PDP context identifier
  -waits the registration to the server LWM2M
  -turns on the GPS writing the value 1 in 33211/0/0/0 through AT#LWM2MW=0,33211,0,0,0,1
  -verifies the GPS configuration with AT$GPSCFG?
  -switches the module's priority to GNSS priority with AT$GPSCFG=3,0
  -waits for the GPS fix
  -switches module's priority to WWAN priority with AT$GPSCFG=3,1
  -waits 30 seconds in WWAN priority to allow the data sending to the server
  -turns off the GPS writing the value 0 in 33211/0/0/0 with AT#LWM2MW=0,33211,0,0,0,0
  -disables the client with AT#LWM2MENA=0
*/
void loop() {

  int r1;

  if (isr == true) {
    bma400.read0x0e();
    //enable the LWM2M client
    Serial.println("Enabling and registering LWM2M client");
    r1 = myME310.enableLWM2M(1, 1, ME310::TOUT_45SEC);

    if (r1 == ME310::RETURN_VALID)
    {
      delay(20000);
      Serial.println("Registered on server DM");
    }
    else
    {
      Serial.println("Enabling LWM2M returned error");
    }


    //turn on GPS
    Serial.println("Writing 1 in resource 33211/0/0/0");
    myME310.writeResource(0, 33211, 0, 0, 0, 1, ME310::TOUT_20SEC);
    delay(3000);

    //verify gps configuration
    ME310::return_t r = myME310.read_gnss_configuration();
    Serial.println(myME310.return_string(r));
    Serial.println(myME310.buffer_cstr(1));
    if (r == ME310::RETURN_VALID)
    {
      delay(100);
      //switch to GNSS priority
      Serial.println("Switching to GNSS priority");
      r = myME310.gnss_configuration(3, 0, ME310::TOUT_30SEC);
      if (r == ME310::RETURN_VALID)
      {
        Serial.println("Switched to GNSS");
      }
      else
      {
        Serial.println("Couldn't switch to GNSS");
      }
      /*While loop in which the MCU issue the command AT#LWM2MR=0,6,0,3
         if the value f is 0.0< f < 20.00000, exit form while loop and set radius to true
         else continue to issue the command AT#LWM2MR every 10 seconds
      */
      while (radius == false)
      {
        myME310.readResourcefloat(0, 6, 0, 3, 0, ME310::TOUT_10SEC);
        resp = myME310.buffer_cstr(1);
        if(resp != NULL)
        {
          char buffFloat[] = {resp[9], resp[10], resp[11], resp[12], resp[13], resp[14], resp[15], resp[16], resp[18], resp[19]};
          //Serial.println(buffFloat);
          float f = atof(buffFloat);
          if (f > 0.0 && f < 20.00000)
          {
            radius = true;
          }
          else
          {
            //do nothing
          }
          delay(10000);
        }
      }
      if (radius == true)
      {

        //switch to WWAN priority
        r = myME310.gnss_configuration(3, 1, ME310::TOUT_30SEC);
        if (r == ME310::RETURN_VALID)
        {
          Serial.println("Switched to WWAN");
        }
        else
        {
          Serial.println("Couldn't switch to WWAN");
        }
        myME310.setResourceBool(0, 3200, 0, 5500, 0, 1, ME310::TOUT_30SEC);
        delay(40000);

        //turn off the GPS
        Serial.println("Writing 0 in resource 33211/0/0/0");
        r = myME310.writeResource(0, 33211, 0, 0, 0, 0, ME310::TOUT_20SEC);
        delay(2000);
        // The following line is used to restore the variable isr to false to do the sketch' test.
        //If you don't need to do testing, please comment the following line with //
        myME310.setResourceBool(0, 3200, 0, 5500, 0, 0, ME310::TOUT_20SEC);

        delay(2000);
      }
      delay(2000);
      Serial.println("Disabling client");
      r = myME310.disableLWM2M(0, ME310::TOUT_10SEC);
      if (r == ME310::RETURN_VALID)
      {
        Serial.print("Client disabled");
      }
      exit(0);
    }
  }
  else {
    Serial.println("Freefall not occurred");
  }
  delay(2000);
}
