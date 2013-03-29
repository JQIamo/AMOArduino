/* 
   SimpleLCD.h - wrapper library for interfacing with SparkFun SerialLCD
   Created by Neal Pisenti, 2013.
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

#ifndef SimpleLCD_h
#define SimpleLCD_h


#include "Arduino.h"

class SimpleLCD
{
    public: 
        SimpleLCD(HardwareSerial * lcd);

        void write(char*);
        void write(int);
        void write(double);
        void write(int, double);
        void write(int, int);
        void write(int, char*);



        void clearScreen();
        void clearLine(int);
        void selectLine(int);
        void scrollRight();
        void scrollLeft();
        void displayOff();
        void displayOn();
        void underlineCursorOn();
        void underlineCursorOff();
        void boxCursorOn();
        void boxCursorOff();
        void backlight(int);

        void setDecimalCount(int);
        

    private:
        HardwareSerial* _lcd;
        int _decimalPlaces;

};

#endif
