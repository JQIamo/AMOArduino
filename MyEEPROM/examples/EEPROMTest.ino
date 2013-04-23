#include <Wire.h>
#include <MyEEPROM.h>



void setup(){
  byte msg[] = {1,2,5};
  Wire.begin();
  Serial.begin(9600);
  
  Serial.println("Writing to 0x100");
  MyEEPROM.write(0x100, msg, 3);
 
}

void loop(){
  byte buffer[3];
  Serial.println("Reading from 0x100");
  MyEEPROM.read(0x100, buffer, sizeof(buffer));
  for (int i = 0; i < sizeof(buffer); i++){
  
    Serial.println(buffer[i]);
  }
  
  delay(5000);
}
