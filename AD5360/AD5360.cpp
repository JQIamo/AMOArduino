/*
   AD5360.cpp - AD5360 DAC control library 
   Created by Alessandro Restelli, 2013
   JQI - Joint Quantum Institute

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

// include core Arduino API
#include "Arduino.h"

// include this library's header
#include "Test.h"

// additional header files if used

//The AD5360 uses SPI interface
#include "SPI.h"

// Constructor
// some parameters related to the particular hardware implementation

//The physical pin used to enable the SPI device
#define SPI_DEVICE 4

//The clock frequency for the SPI interface
#define AD5360_CLOCK_DIVIDER_WR SPI_CLOCK_DIV2
#define AD5360_CLOCK_DIVIDER_RD SPI_CLOCK_DIV4 //that (Assuming and Arduino clocked at 80MHz will set the clock of the SPI to 20 MHz
											   //AD5360 can operate to up to 50 MHz for write operations and 20MHz for read operations.





AD5360::AD5360(void)
{
  /*not really sure what to put here
  It should be possibile to call the initialization of the SPI port by uncommenting the followingrow */

//initialize_SPI(void);

	
	
	
}






// Public Methods
// Function used by this sketch and external sketches 

AD5360::initialize_SPI(void)
{

//First let us chose the channel	
	SPI.begin(SPI_DEVICE);
	//Initialize the bus for a device on pin SPI_DEVICE
	SPI.setClockDivider(SPI_DEVICE,AD5360_CLOCK_DIVIDER_WR);
	SPI.setDataMode(SPI_DEVICE,SPI_MODE1);// this setting needs to be double checked.
	//Set between little endian and big endian notation
	SPI.setBitOrder(SPI_DEVICE,MSBFIRST);
	
}



void AD5360::write(void)
{

	
	
}

// Private Methods
// Functions used only in this library and not accessibile outside. 

//void AD5360::function(void)
{
  
}

