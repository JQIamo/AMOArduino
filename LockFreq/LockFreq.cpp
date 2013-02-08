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
//      baseFreq: "baseline" frequency, in hertz
//      feedbackPin: analog pin # (ie, A0, A1, A2, etc.) where the arduino should
//          look for a voltage input.
LockFreq::LockFreq( unsigned long baseFreq, int feedbackPin){
    _baseFreq = baseFreq;
    _feedbackPin = feedbackPin;

};

// initialize(dds) -- initialize function. 
//      dds: an AD9954 DDS object, which will be synthesizing frequencies.
void LockFreq::initialize(AD9954& dds){

    // Assigns the _dds pointer to our DDS
    _dds = &dds;

    // Configures arduino for maximum analog resolution, and sets up analog input pin
    analogReadResolution(12);
    pinMode(_feedbackPin, INPUT);

    // Reads out initial offset from baseFreq.
    // For the time being, the offset is bit-shifted by 8 bits, meaning
    // 12-bit ADC -> 4096 "points". Bit shifting these by a factor of 2^8 = 256
    // gives a dynamic range of 1Mhz, with resolution 256 Hz. However, to minimize
    // jitter in the ADC, the "update frequency" function checks if the new analog 
    // voltage value is more than 2 (out of 4096) different from previous voltage.
    // Thus, our real resolution is 2*256 = 512 Hz.
    _offset = analogRead(_feedbackPin) << 8;
    _dds->setFreq(_baseFreq + _offset);

}


// updateFreq() -- updates the frequency output of the DDS by polling the 12-bit ADC voltage
//      on our feedback pin.
//  See note above, re: translation to frequency resolution!
void LockFreq::updateFreq(){
    int tempOffset = analogRead(_feedbackPin) << 8;
    if ((_offset - tempOffset) >= (2 << 8)){
        _offset = tempOffset;
        _dds->setFreq(_baseFreq + _offset);    
    }
}

