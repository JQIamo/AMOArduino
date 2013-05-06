# SimpleLCD

This is a wrapper class for writing messages to the SparkFun serial LCD display, using the Arduino (and specifically the Arduino Due, which
does not need the SoftwareSerial class).

## Getting Started

First you'll need to do something like

    #include <SimpleLCD.h>

    SimpleLCD LCD(&Serial1);    // use the Serial1 line on Arduino Due

    void setup(){

        Serial1.begin(9600); // start serial connection on Serial1 line, at 9600 baud

        LCD.clearScreen();
        LCD.write(1, "Initializing...");    // write "Initializing..." to line 1

    }

    void loop(){

        LCD.write(1, "Line 1"); // clears line 1, writes "Line 1" from position 0.
        LCD.write(" (first line)"); // continues writing on line 1 from last cursor position

        LCD.write(2, "Line 2");
        LCD.write(" (second line)");

    }

## Implemented Features

Scroll through SerialLCD.cpp, there are a number of functions I more or less copied out of the SparkFun examples; they should be 
pretty self-explanatory. But, there are a few new/handy ones too...

    SimpleLCD::write(int line, text);
    SimpleLCD::setDecimalCount(int places); // for writing floating point numbers,
                                            // will truncate the decimal at the desired precision.


Have fun!
