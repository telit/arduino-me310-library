/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    ME310.cpp
    string.h
    stdio.h

  @brief
    Driver Library for ME310 Telit Modem

  @details
    The library contains a single class that implements a C++ interface to all ME310 AT Commands.
    It makes it easy to build Arduino applications that use the full power of ME310 module

  @version 
    1.0.0
  
  @note

  @author
    BlackIoT Sagl

  @date
    28/10/2020
 */
 
 #include <ME310.h>

/*  
    //sercom2
    #define PIN_MODULE_RX 29
    #define PIN_MODULE_TX 28
    #define PAD_MODULE_TX UART_TX_PAD_0
    #define PAD_MODULE_RX SERCOM_RX_PAD_1
    #define PIN_MODULE_RTS 30
    #define PIN_MODULE_CTS 31

    #define ON_OFF 27
    
    Uart SerialModule(&sercom4, PIN_MODULE_RX, PIN_MODULE_TX, PAD_MODULE_RX, PAD_MODULE_TX, PIN_MODULE_RTS, PIN_MODULE_CTS);

    ME310 myME310 (SerialModule); 
    
 */

using namespace me310;
 
ME310 myME310; 

void print_buffer(ME310 &aME310, const char *term = "OK");

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

  Serial.println("SERCOMM Telit Test AT");
  
  turnOnModule();
  
  Serial.println("ME310 ON");

  Serial.println();
   Serial.println("AT Command");
   ME310::return_t rc = myME310.attention();    // issue command and wait for answer or timeout
   Serial.println(myME310.buffer_cstr());       // print first line of modem answer
   Serial.print(ME310::return_string(rc));      // print return value
   Serial.println(" answer from ME310 MODULE");  
   if(rc != ME310::RETURN_VALID)                // exit on error
      return;     


   Serial.println();
   Serial.print("Soft Reset Command : ");
   rc = myME310.soft_reset ();                 // issue command and wait for answer or timeout
   Serial.println(ME310::return_string(rc));   // print return value
   if(rc != ME310::RETURN_VALID)               // exit on error
      return;     
   
   Serial.println();
   Serial.println("Display Config Profile : ");  
   rc = myME310.display_config_profile();      // issue command and wait for answer or timeout
   if(rc == ME310::RETURN_VALID)               // print all rows returned from ME310 except command echo (index = 0)
      print_buffer(myME310);
   else return;                                // exit on error
   
   Serial.println();
   Serial.print("Query SIM Status : ");  
   myME310.query_sim_status();
   Serial.println(myME310.buffer_cstr(1));

   Serial.println();
   Serial.println("Read Query SIM Status : ");  
   rc = myME310.read_query_sim_status();
   if(rc == ME310::RETURN_VALID)               // print all rows returned from ME310 except command echo (index = 0)
   {
      Serial.println(myME310.buffer_cstr(1));
      if(strstr(myME310.buffer_cstr(1),"#QSS: 0,1"))
      {
         Serial.println("SIM is inserted");
         
         rc = myME310.read_enter_pin();
         if(rc == ME310::RETURN_VALID)               // print all rows returned from ME310 except command echo (index = 0)
         {
            if(strstr(myME310.buffer_cstr(1),"READY"))
               Serial.println("PIN not required");
            else
               Serial.println("PIN required");
         }
         else return;      

         Serial.println();
         Serial.print("Print ICCID : ");  
         rc = myME310.read_iccid();
         if(rc == ME310::RETURN_VALID)
         {
            const char *pLabel = strstr(myME310.buffer_cstr(1),"+CCID: ");
            if(pLabel) 
            {
               Serial.print("[");
               Serial.write(pLabel+7,19);
               Serial.println("]");
            }
         }
         else return;
         
         Serial.print("Print IMSI : ");  
         rc = myME310.imsi();                 // no command echo
         if(rc == ME310::RETURN_VALID)
            Serial.println(myME310.buffer_cstr(1));
         else return;
         
         Serial.println();
         Serial.println("List Capabilities : ");  
         rc = myME310.capabilities_list();
         if(rc == ME310::RETURN_VALID)               // print all rows returned from ME310 except command echo (index = 0)
            Serial.println(myME310.buffer_cstr(1));
         else return;   

      }
      else
         Serial.println("SIM not inserted");
   }
   else return;                                // exit on error


   Serial.println();
   Serial.print("Manufacturer Identification : ");  
   rc = myME310.manufacturer_identification();
   if(rc == ME310::RETURN_VALID)               
      Serial.println(myME310.buffer_cstr(1));
   else return;   
   
   Serial.print("Model Identification        : ");  
   rc = myME310.model_identification();
   if(rc == ME310::RETURN_VALID)               
      Serial.println(myME310.buffer_cstr(1));
   else return;   

   Serial.print("Revision Identification     : ");  
   rc = myME310.revision_identification();
   if(rc == ME310::RETURN_VALID)               
      Serial.println(myME310.buffer_cstr(1));
   else return;   

   Serial.print("Serial Number               : ");  
   rc = myME310.serial_number();
   if(rc == ME310::RETURN_VALID)               
      Serial.println(myME310.buffer_cstr(1));
   else return;   

   Serial.print("Request Manufacturer Ident. : ");  
   rc = myME310.request_manufacturer_identification();
   if(rc == ME310::RETURN_VALID)               
      Serial.println(myME310.buffer_cstr(1));
   else return;   
   
   Serial.print("Request Model Ident.        : ");  
   rc = myME310.request_model_identification();
   if(rc == ME310::RETURN_VALID)               
      Serial.println(myME310.buffer_cstr(1));
   else return;   

   Serial.print("Request Revision Ident.     : ");  
   rc = myME310.request_revision_identification();
   if(rc == ME310::RETURN_VALID)               
      Serial.println(myME310.buffer_cstr(1));
   else return;   

   Serial.print("Product Serial Number       : ");  
   rc = myME310.request_psn_identification();
   if(rc == ME310::RETURN_VALID)               
      Serial.println(myME310.buffer_cstr(1));
   else return;   
   
   Serial.print("Product Code                : ");  
   rc = myME310.request_product_code();
   if(rc == ME310::RETURN_VALID)               
      Serial.println(myME310.buffer_cstr(1));
   else return;   
   
   Serial.println();  
   Serial.println("Software Package Version    : ");  
   rc = myME310.request_software_package_version();
   if(rc == ME310::RETURN_VALID)               
      print_buffer(myME310);
   else return;   
   
}

void loop() {
  Serial.println();
  Serial.println("Transparent Bridge Started");
  while(1){
    while (SerialModule.available()) {
      Serial.write(SerialModule.read());
    }
    while (Serial.available()) {
      SerialModule.write(Serial.read());
    }
  }
}

void print_buffer(ME310 &aME310, const char *term)
{
   for(int index = 1;;index++)
   {
      const char * tmp = aME310.buffer_cstr(index);
      if(tmp) 
      {
        if(term)
        {
           if(strcmp(tmp,term))
              Serial.println(tmp);         // print if not null  
           else
              break;   
        }
      }
      else break;                          // exit loop if null
   }
}
