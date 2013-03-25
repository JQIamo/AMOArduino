/* 
   LockFreq.h - DDS frequency locking class
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

#ifndef LockFreq_h
#define LockFreq_h

#include "Arduino.h"


class LockFreq {

    public:
        LockFreq(int, int, int);


        void updateFreq();
        void initialize(AD9954&, unsigned long);
        void updateBaseFreq(unsigned long);


    private:
        unsigned long _baseFreq, _baseFTW, _offset;
        int _offsetVoltage;
        int _feedbackPin, _res;
        int _adMultiplier;
        AD9954* _dds;

};

#endif
