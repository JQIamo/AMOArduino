#include <Wire.h>
#include <MyEEPROM.h>



void setup(){
  byte msg[] = {1,2,5};
  byte mystr[] = "testing yo!";
  Wire.begin();
  Serial.begin(9600);
  
  Serial.println("Writing to 0x100");
  MyEEPROM.write(0x100, msg, 3);

  Serial.println("Writing to 0x200");
  MyEEPROM.write(0x100, mystr, sizeof(mystr));

 
}

void loop(){
  byte buffer[3];
  byte buffer2[16];
  Serial.println("Reading from 0x100");
  MyEEPROM.read(0x100, buffer, sizeof(buffer));

  for (int i = 0; i < sizeof(buffer); i++){
    Serial.println(buffer[i]);
  }
  
  Serial.println("Reading from 0x200");
  MyEEPROM.read(0x200, buffer22, sizeof(buffer2));

  for int(i=0; i<sizeof(buffer2); i++){
      Serial.write(buffer2[i]);
  }
  delay(5000);
}
