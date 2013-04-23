/* Code for using the I2C bus with 24LC16B EEPROM chip.

MyEEPROM.h - Code to interface with external EEPROM on the Arduino

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

#ifndef MyEEPROM_h
#define MyEEPROM_h

#include "Arduino.h"

class MyEEPROMClass {

    public:

        void write(int, const byte*, byte);
        byte read(int, byte*, byte);



};

extern MyEEPROMClass MyEEPROM;

#endif
