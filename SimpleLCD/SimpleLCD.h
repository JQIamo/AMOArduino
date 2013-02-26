#ifndef SimpleLCD_h
#define SimpleLCD_h


#include "Arduino.h"

class SimpleLCD
{
    public: 
        SimpleLCD(HardwareSerial)

        void write(char);
        void clearScreen();
        void selectLine(int);
        void scrollRight();
        void scrollLeft();
        void displayOff();
        void displayOn();
        void underlineCursorOn();
        void underlineCursorOff();
        void boxCursorOn();
        void boxCursorOff();
        int backlight(int);

