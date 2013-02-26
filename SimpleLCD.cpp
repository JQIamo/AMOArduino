/*
   SimpleLCD.cpp - wrapper library for interfacing with SparkFun SerialLCD
   Created by Neal Pisenti, 2013
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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */


#include "Arduino.h"
#include <SimpleLCD.h>

/* CONSTRUCTOR */

// Constructor function. 
//      lcd : a hardware serial object -- ie, Serial, Serial1, Serial2, etc.
SimpleLCD::SimpleLCD(HardwareSerial lcd){

    _lcd* = &lcd;
    // Begin serial interface...
    _lcd->begin(9600);
    delay(500);
    SimpleLCD::clearScreen();
}

/* PUBLIC METHODS */

void SimpleLCD::clearScreen(){
    _lcd->write(0xFE);
    _lcd->write(0x01);
} 

// selectLine: moves cursor to beginning of specified line.
//      line: int, either 1 or 2
void SimpleLCD::selectLine(int line){
    if (line == 1){
        _lcd->write(0xFE);
        _lcd->write(0x128);
    } elsif(line == 2){
        _lcd->write(0xFE);
        _lcd->write(0x192);
    }
}

void SimpleLCD::scrollRight(){
        _lcd->write(0xFE);
        _lcd->write(0x20);
}

void SimpleLCD::scrollLeft(){
        _lcd->write(0xFE);
        _lcd->write(0x20);
}

void SimpleLCD::displayOff(){
    _lcd->write(0xFE);
    _lcd->write(0x08);
}

void SimpleLCD::displayOff(){
    _lcd->write(0xFE);
    _lcd->write(0x0C);
}

void SimpleLCD::underlineCursorOn(){
    _lcd->write(0xFE);
    _lcd->write(0x0E);
}

void SimpleLCD::underlineCursorOff(){
    _lcd->write(0xFE);
    _lcd->write(0x0C);
}

void SimpleLCD::boxCursorOn(){
    _lcd->write(0xFE);
    _lcd->write(0x0D);
}

void SimpleLCD::boxCursorOff(){
    _lcd->write(0xFE);
    _lcd->write(0x0C);
}

// brightness takes avalue between 128 and 157
// 128 is OFF, 157 is FULLY ON
// brightness on the LCD is implemented with a PWM signal
void SimpleLCD::backlight(int brightness){
    _lcd->write(0x7C);
    _lcd->write(brightness);
}


void write(char text){
    _lcd->print(text);
}
