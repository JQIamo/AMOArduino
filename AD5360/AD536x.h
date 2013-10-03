/*
   AD536x.h  - AD536x family DAC control library 
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


   This library works for both the AD5360 and AD5361 chips.
   To properly use the library with AD5361 it is imperative to set the two LSB to 0
   Setting those values to 1 is deprecated because those bits are reserved for future use in the AD5361

*/

// ensure this library description is only included once
#ifndef Test_h
#define Test_h

// include types & constants of Wiring core API
#include "Arduino.h"


//Definition of constants and AD5360 registers
/*

AD5360 (AD5361) has a 24 bit instruction/data frame organized like this:

23	22	21	20	19	18	17	16	15	14	13	12	11	10	9	8	7	6	5	4	3	2	1		0
M1	M0	A5	A4	A3	A2	A1	A0	D15	D14	D13	D12	D11 D10 D9	D8	D7	D6	D5	D4	D3	D2	D1(0)	D0(0)

*/


// First I will define a table with the 2 the first 2 control bits

#define AD5360_SPECIAL_FUNCTION 0
#define	AD5360_WRITE_DAC 3<<22
#define AD5360_WRITE_OFFSET 2<<22
#define	AD5360_WRITE_GAIN 1<<22

//Then I will define constats used to generate the address or the special codes.

//Whem AD5360_SPECIAL_FUNCTION is not used address bits A4 and A3 are important to select which bank of DAC to control.
// A5 is not in use and must be always set to 0

#define AD5360_BANK0		1<<18
#define AD5360_BANK1		2<<18
#define AD5360_BANK0_ALL	0
#define AD5360_BANK1_ALL	1<<15
#define AD5360_ALL_DACS		2<<15

//if BANK0 or BANK1 are selected, then A2 to A0 are used to address the particular DAC within a bank
//The expression {variable}<<15 will care of the correct format of this part of the address
//IF ALL_DACS BANK0_ALL or BANK1_ALL are selected then bits A2 and A0 are already taken care.

//Whem AD5360_SPECIAL_FUNCTION is used it is possibile to give complex commands to the AD5360
//including reading from registers!
//In Special function mode the frame is organized in this way:

// 23	22	21	20	19	18	17	16	15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0
//  0	 0	S5	S4	S3	S2	S1	S0	F15	F14	F13	F12	F11 F10 F9	F8	F7	F6	F5	F4	F3	F2	F1	F0

#define AD5360_NOP	0
#define AD5360_WR_CR 1<<15 //Write on control register
	//Let's set the FLAGS for the control register
	//Flags can be combined!!!
	#define AD5360_X1B 4
	#define AD5360_X1A 0
	#define AD5360_T_SHTDWN_EN 2
	#define AD5360_T_SHTDWN_DIS 0	
	#define AD5360_SOFT_PWR_UP 1	
	#define AD5360_SOFT_PWR_DWN 0	
#define AD5360_WR_OFS0 2<<15 //Writes in the OFFSET 0 ANALOG DAC. the data is a 14 bit variable
#define AD5360_WR_OFS1 3<<15 //Writes in the OFFSET 1 ANALOG DAC. the data is a 14 bit variable
#define AD5360_READ_REG 5<<15 //Select which register to read
	//This is the set of commends that select a particular register
	#define AD5360_READ_X1A(channel) 0|(channel+8)<<6
	#define AD5360_READ_X1B(channel) (1<<12)|(channel+8)<<6
	#define AD5360_READ_C(channel) (2<<12)|(channel+8)<<6
	#define AD5360_READ_M(channel) (3<<12)|(channel+8)<<6
	#define AD5360_READ_CR (3<<12)|(1<<6) //Read the control register: my favorite!
		//Flags defined for register Writing can be used for interrogation of the state
		//In addition the following flags can be used for read-only interrogations
		#define AD5360_CR_OVERTEMP 16 
		#define AD5360_CR_PEC 8
	#define AD5360_READ_OFS0 (3<<12)|(2<<6)						 
	#define AD5360_READ_OFS1 (3<<12)|(3<<6)							 
	#define AD5360_READ_AB_0 (3<<12)|(6<<6)
	#define AD5360_READ_AB_1 (3<<12)|(7<<6)	 
	#define AD5360_READ_GPIO (3<<12)|(11<<6) //F6 to F0 SHOULD be 0
#define AD5360_WR_AB_SELECT_0 6<<15 //F7 to F0 select registers X2A or X2B for bank 0 A is0 and B is 1
#define AD5360_WR_AB_SELECT_0 11<<15 //F7 to F0 select registers X2A or X2B for bank 0 A is0 and B is 1
#define AD5360_BLOCK_WR_AB_SELECT 19<<15 //Block write AB
#define AD5360_MON 			12<<15 //Additional monitor commands specified below
	#define AD5360_CMD_MON_ENABLE 1<<4
	#define AD5360_CMD_MON_DISABLE 1<<4
	#define AD5360_CMD_MON_IN_PIN_SEL(pin) (1<<4)|pin // pin can be 0 or 1 and selects the input pin from MON_IN0 or MON_IN1								 
	#define AD5360_CMD_MON_DAC_CH_SEL(channel) channel // F3:F0 selects the input channel. If a number greater than 15 is used will cause errors!
#define	AD5360_WR_GPIO 13<<15 //F1=1 sets the GPIO as output F1=0 sets GPIO as input F0 contains the status.



// library interface description
class AD5360
{
  // user-accessible "public" interface
  public:
  	
    void write(unsigned long int command);
    void doSomething(void);

  // library-accessible "private" interface
  private:
    //int frame;
    //void function(void);
};

#endif

