/* 
   LockFreq.cpp - DDS frequency locking class
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

#include "Arduino.h"
#include "AD9954.h"
#include <LockFreq.h>

/* CONSTRUCTOR */

// LockFreq -- constructor function. 
//      feedbackPin: analog pin # (ie, A0, A1, A2, etc.) where the arduino should
//          look for a voltage input.
//      res: integer, "resolution" of ADC, ie, what change in ADC value constitues a "new" frequency?
//      multiplier: integer, representing bitshift in going from ADC value to frequency
LockFreq::LockFreq(  int feedbackPin, int res, int multiplier ){
    _feedbackPin = feedbackPin;
    _res = res;
    _adMultiplier = multiplier;

    // Configures arduino for maximum analog resolution, and sets up analog input pin
    analogReadResolution(12);
    pinMode(_feedbackPin, INPUT);
};

// initialize(dds) -- initialize function. 
//      dds: an AD9954 DDS object, which will be synthesizing frequencies.
// mulitplier is an integer, which sets the bitshift from pot value << multiplier to get frequency offset (in Hz).
// eg, pot value 100, multiplier 8 = 2^8 = 256 means frequency offset
// 25.6 kHz
// use this in combination with the AD "res" parameter to figure out your
// channel spacing (ie, digital pot values are spaced by pot "res")
//      baseFreq: "baseline" frequency, in hertz

// If the frequency freq is zero, it initializes from the POT value. otherwise, initializes to given frequency
void LockFreq::initialize(AD9954& dds, unsigned long baseFreq, unsigned long freq){
    // Assigns the _dds pointer to our DDS
    _dds = &dds;
    _baseFreq = baseFreq;



    if (freq == 0){
        // if specified frequency is zero, reads POT value
        _offsetVoltage = analogRead(_feedbackPin);
        _offset = _offsetVoltage << _adMultiplier;
    } else{
        // else, sets DDS frequency to _baseFreq + freq.
        // the user can, of course, update this by calling updateFreq() to poll the POT voltage.
        _offsetVoltage = 0;
        _offset = freq;
    }
    _dds->setFreq(_baseFreq + _offset);
    
}



// updateFreq() -- updates the frequency output of the DDS by polling the 12-bit ADC voltage
//      on our feedback pin.
//  See note above, re: translation to frequency resolution!
void LockFreq::updateFreq(){
    int offsetVal = analogRead(_feedbackPin);
    if (abs(_offsetVoltage - offsetVal) >= _res){
        _offsetVoltage = offsetVal;
        _offset = _offsetVoltage << _adMultiplier;
        _dds->setFreq(_baseFreq + _offset);    
    }
}

// updateBaseline(baseFreq) -- updates the "base" frequency (ie, frequency from which the feedback pin voltage
//      calculates its offset from...)
void LockFreq::updateBaseFreq(unsigned long baseFreq){
    _baseFreq = baseFreq;
}

void LockFreq::updateCenterFreq(unsigned long centerFreq){
    _baseFreq = centerFreq -  ((2048/_res) << _adMultiplier);
}

unsigned long LockFreq::getSetpoint(){
    return analogRead(_feedbackPin) << _adMultiplier;
}
