/* Code for using the I2C bus with 24LC16B EEPROM chip.

MyEEPROM.cpp - Code to interface with external EEPROM on the Arduino

Adapted from http://www.fact4ward.com/blog/ic-if/arduino-24c16/

Current version by Neal Pisenti, 2013
JQI - Strontium - UMD

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   aunsigned long with this program.  If not, see <http://www.gnu.org/licenses/>.


*/

#include "Arduino.h"
#include <MyEEPROM.h>
#include "Wire.h"

// Write to EEPROM. The 24LC16B has 8 blocks of 256 bytes.
// Thus, the memAddress needs 3 bits to specify the block (MSB), 
// plus another 8 bits to specify the address within that block. 
// Thus, memAddress should be 11 bits.
// Here, we're giving it a type int; you should pass it a hex value, where the
// first digit is the block number (0-8), and the next two digits form the
// byte specifying the memory address. For example,
// 0x020 -- block 0, memory address 32
// 0x500 -- block 5, memory address 0
// etc.

// The *device* address is 1010 (no way to change on 24LC16B) -- these need to 
// be in MSB position --> use 0x50.
// I'm leaving this in as a #define parameter for now.

#define deviceAddress 0x50

// data: an array of bytes of data you'd like to write!
// length: how many bytes you're writing...
// NOTE!! currently limited to reads and writes of (up to) 16 bytes at a time
void MyEEPROMClass::write(int memAddress, const byte * data, byte length){
    byte devAddr = deviceAddress | ((memAddress >> 8) & 0x07); // extracts block, bitwise OR onto device addr.
                                                                // this is the 7-bit address arduino Wire wants.

    Wire.beginTransmission(devAddr); // begin I2C transfer
    Wire.write(memAddress); // specify which memory address you're starting on

    // loops through and writes data; note, will cut you short if you try to write more than 
    // 16 bytes! This preserves the data integrity of what you've already written on the chip.
    for(int i = 0; i < length && i < 16; i++){
        Wire.write(data[i]); // write data
    }
    Wire.endTransmission();
    delay(10); // delay so EEPROM chip can persist buffer to memory


}

byte MyEEPROMClass::read(int memAddress, byte * buffer, byte length){
    byte devAddr = deviceAddress | ((memAddress >> 8) & 0x07); // extracts block, bitwise OR onto device addr.

    // make like you're going to do a write, so EEPROM moves pointer to beginning of readout block
    Wire.beginTransmission(devAddr);
    Wire.write(memAddress);
    Wire.endTransmission(); 

    // request bytes from EEPROM
    Wire.requestFrom(devAddr, length);

    // while bytes left && bytes in Wire RX buffer, add them to the readout buffer.
    // returns the number of bytes read.
    int i;
    for(i = 0; i < length && Wire.available(); i++){
        buffer[i] = Wire.read();
    }
    return i;
}
    
// instantiate class, to use as MyEEPROM in arduino sketches.
MyEEPROMClass MyEEPROM;
