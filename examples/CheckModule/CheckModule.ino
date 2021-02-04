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
 
ME310 myME310; 

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

  myME310.begin(115200);

  delay(1000);
  
  turnOnModule;
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
