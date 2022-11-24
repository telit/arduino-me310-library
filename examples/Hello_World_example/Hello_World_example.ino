/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/**
  @file
    
  @brief
    Sample test of the use the Arduino.

  @details
    In this example sketch, it is shown how to print Hello World.\n
  
  @version
    1.0.0

  @note

  @author
    Cristina Desogus

  @date
    04/20/2022
 */

void setup()
{
    Serial.begin(115200);
    delay(5000);

    Serial.println("Hello World");

}

void loop()
{
  exit(0);
}
