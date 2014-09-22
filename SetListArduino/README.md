# SetListArduino

## Overview

SetListArduino is an Arduino library designed to provide easy integration for
Arduino-controlled devices into the SetList computer control software used at
the JQI. To set up a new Arduino device for SetList control, simply "register"
the device with SetListArduino in your sketch, and provide as many callback 
functions as you need to implement the desired functionality. SetListArduino
orchestrates the communication between LabView's SetList and each registered 
device, executing the appropriate callback function for each new line passed
from SetList.

For examples and more details, see below!

## Example Sketch

Here is a short sketch to illustrate how one might integrate an AD9954 DDS 
board. 

~~~~~~~~~~~~~~~~~~~~{.cpp}
#include <SPI.h>
#include <AD9954.h>
#include <SetListArduino.h>

// Instantiate setlist object.
// Note, it must be called SetListImage for trigger interrupts to work.

#define triggerChannel 13
SetListArduino SetListImage(triggerChannel); 






// Instantiate DDS object, using the AD9954 library.
// Exact instantiation depends on how DDS is connected.
// This could just as easily be any other device the 
// Arduino knows how to communicate with, or even the
// digital/analog outputs of the Arduino itself! 
// SetListArduino is completely device-agnostic, which 
// allows for easy extensibility.

#define D1IOUPDATE 2
#define D1PS1 3
#define D1PS0 4
#define D1OSK 5
#define D1SS 6
#define D1RESET 7

AD9954 DDS(D1SS, D1RESET, D1IOUPDATE, D1PS0, D1PS1, D1OSK);    



// Callback function to set a particular DDS frequency
// We know that LabView will pass a single parameter for the frequency, thus
// we want to pull freq = params[0].
void setDDSFreq(AD9954 * dds, int * params){
    dds->setFreq(params[0]);
}


// Callback function to initiate a DDS ramp.
// Say we set this up so LabView will pass the parameters 
// [freq0, freq1, tau] where tau is the ramp time.
// Here, we do a few calculations, initialize the pins, and call linearSweep
// on the DDS. The Arduino is controlling PS0 to do the sweep; see AD9954
// for details.
void rampDDS(AD9954 * dds, int * params){
    int f0 = params[0];
    int f1 = params[1];
    int tau = params[2];

    double rampRate;

    int delta = f1 - f0;    // calculate delta frequency
    int RR = 1;             // number of SYS_CLK cycles to spend at each
                            // intermediate frequency
    rampRate = ((double) delta)/tau;   // Hz per second
    int posDF = (int)(rampRate * RR * 100E-9);   // calculate posDF for DDS

    // this is implemented in the AD9954 library; see that
    // documentation for details.

    dds->linearSweep(f0, f1, posDF, RR, posDF, RR);
    digitalWrite(D1PS1, HIGH);  // DDS starts sweeping...

}


void setup(){

    Serial.begin(9600);

    SPI.begin();
    SPI.setClockDivider(4);
    SPI.setDataMode(SPI_MODE0);

    DDS.initialize(400000000);  // Start DDS with 400MHz clock

    /*****************************************
        SetListArduino Part!
    ******************************************/

    // Register the DDS device with SetListImage.
    // First argument is device (passed by reference);
    // Second argument is the "channel number", which must match
    // what you entered in LabView. See README for more details.
    SetListImage.registerDevice(DDS, 0);

    // Register as many callback functions as you need...
    // This says the short-command "f" on channel 0 should execute
    // the callback function setDDSFreq().
    SetListImage.registerCommand("f", 0, setDDSFreq);

    // Here is another callback function, providing a method to 
    // execute DDS ramps. Note, the short-command is different, but
    // we still want this functionality for our DDS on channel 0.
    // If we had a second DDS, say on channel 1, we would have to do a 
    // second SetListImage.registerCommand("r", 1, rampDDS);
    SetListImage.registerCommand("r", 0, rampDDS);



}

void loop(){
    SetListImage.readSerial();  // listen for & process serial commands 
                                // from computer
}
~~~~~~~~~~~~~~~~~~~~







# Temporary Notes; clean up and integrate!

* can't have an arduino-controlled ramp line on first line of setlist

Why? because currently it should put the devices into the state of the first setlist line as soon as labview calls "$". However, check that this is actually implemented in the library code. Also maybe think of a better way to trigger the start of a run, so this isn't a problem...


## Original Description

Basic definitions of what I want to implement:

Arduino devices are triggered by Slaved Digital channels from SetList.
This means they get at most one trigger per line. As envisioned, SetList
will only provide a trigger pulse when the Arduino needs to step to the next 
output value. 

Currently, we have the Arduino controlling a few different devices in the
lab.

* DDS boards
  * Output single-tone frequencies
  * Output linear frequency sweeps
  * More complex functionality can be implemented (see AD9954 library).
* AD536x DACs
  * Library still needs some polishing; control 8- or 16-channel DAC chips from Analog Devices.
  * Again, could output single voltage or interpolated sweep. However, the intention here is to use these for analog channels that don't need updating during a cycle.
	  
All of this to say, what functionality should the SetList library include?

The SerialCommand library provides a nice hook for delegating serial
commands to specific functions. Either re-factor this into SetListArduino,
or incorporate it. Need to figure out exactly how the "Native USB" arduinos
handle serial communication, or if there is still some optimization that can
be done?

Ok, here's a swing at some software definition use cases:

In an Arduino sketch, the user should be able to define an array of 
"controlled devices" which the Arduino is connected to.

For each controlled device, the user should be able to map "short commands"
from the serial interface to setter methods on the device. The arguments
(see eg SerialCommand library) should come in the same order as required by 
the device setter function.

In LabView, the user should be able to parse the SetList table into a list
of serial commands to send to the Arduino. A block of serial commands should
be prefaced by "@ (int) DeviceUID (int) numCommands", where the DeviceUID 
corresponds to an array index of the controlled devices array, and
numCommands corresponds to the number of commands it is expecting.

Once LabView specifies the device and number of commands it will send, it 
loops through and sends each command.

After the commands have been sent, LabView should indicate the device should
be initiated. Arduino will program that device to the state given by the
first command, and wait for the first falling edge of its Slaved Digital
line to begin stepping through a command list.

LabView should require the Slaved Digital line to be high on the first step
of a cycle; this way, Arduino can wait for a falling edge to know the cycle
has started. Arduino will then switch interrupt routines to listen for a
change on that pin, and call the setter method for each of the controlled
devices as necessary.

Question: for an Arduino controlling multiple devices, do we use multiple 
Slaved Digital pseudo-triggers, or somehow negotiate the updating with a
single trigger line?

Here's a better idea for a more general SetListArduino device:

* LabView implements the class Device > Arduino, which has variable number of "channels," each of which can have a variable number of Columns (regular or digital) configurable by the user.
* One of these columns can specify a short command; eg, "r" for ramp or "f" for frequency output. If only one type of behavior is allowed (eg, "voltage" output), then user can configure a default short command.
* LabView's Arduino class 
* The user, in setting up the device, now just has to negotiate command mappings to controlled devices' setter functions. eg:
  
SetListArduino.registerCommand("f", channelNum, &setFreq, argumentNum);
  
Now, upon encountering short command "f", SetListArduino would parse out
however many arguments are expected by argumentNum, pass that to setFreq,
and apply it to the controlled device in the specified channelNum.

To deal with timing issues, we might want to implement a "delayed update"
functionality, ie, using the I/O update pin on the DAC or similar. Thus, 
upon receiving a trigger, the Arduino immediately propagate an I/O update 
on each of the devices, then write the n+1 instruction in the SetList to
prep for the next output.

This might be facilitated by having an additional reserved short command,
"h", to hold previous value (ie, don't trigger the I/O update).

It would be nice too if we had a manual controls equivalent -- must ask 
Creston or Zach how manual controls is integrating new devices.

But, could consider a situation where "update hardware" in manual controls
puts the Arduino in a different state, where it instantaneously updates
the output of a given channel, eg, with the command "@m".
For example, update hardware -> "@m", then the sequence of commands
eg, f 1 100; r 2 100 200 ...; etc. After each command is received, it 
knows to update NOW rather than wait for a triggered pulse. To put the 
Arduino back into cycle control, it would just wait for the negotiation of
a new SetList via "@ (int)channelNum (int)numCommands", etc.

This hopefully catches a pretty broad swath of uses, and will be
customizable enough to be reusable.
  

