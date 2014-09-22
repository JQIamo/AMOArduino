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

## Limitations & Important Notes

There are a few `#define` statements in `SetListArduino.h` to be aware of.
If you want to overwrite these, do so before including `SetListArduino.h` in
your Arduino sketch.

* `#define MAX_SETLIST_LINES 512` -- SetList array size; don't make a setlist with more than 512 lines!
* `#define MAX_DEVICE_NUMBER 6` -- deviceList array size. If you want more than 6 devices connected to a single Arduino, you'll need to increase this.
* `#define MAX_PARAM_NUM 8` -- Parameter array size. This limits the number of parameters that will be parsed from the serial stream; if you need more than 8, be sure to redefine this!
* `#define SERIALCOMMAND_BUFFER 512` -- Buffer length for serial commands. This limits a single serial line to 512 characters, which should hopefully be enough.
* `#define SERIALCOMMAND_MAXCOMMANDLENGTH 8` -- Max length of "short command." But to be economical, you should try to limit your short commands to a single character.
* `#define SETLIST_ERROR_CHECK 1` -- Sends error info back to LabView
* `#define SETLIST_DEBUG` -- If you want debug info printed back to the serial stream, make sure to define this.

### Device Channels - Important!

The "device channel" (see `registerDevice()` below) corresponds directly to an
index in _deviceList. Thus, make sure your channels start at 0 and are
sequential. If you don't do this, the Arduino may segfault as it tries to 
execute functions in memory that has not been properly initialized.

### No "ramp" on first SetList line

Because of idiosyncrasies in how we decided to trigger the Arduino, 
SetListArduino will immediately initialize all devices with the callback
given on the first line, and wait for a falling edge trigger to move to line #2.
Thus, if you have some non-static output behavior on line #1, it will begin
before the official "start" of the cycle.

The reason for this triggering behavior is that SetList (LabView edition) passes
a single state per SetList line to a slaved digital channel on the PulseBlaster;
if your SetList has an odd number of lines, you might end in the same state as 
the first SetList line, and thus not actually send a trigger for the first line.

Ultimately, this is a bug which should be fixed.

### SetListImage vs SetListArduino

`SetListArduino` is the name of the class which orchestrates the SetList
computer control, while `SetListImage` is the instance name you want to use.

Doing it this way allows us to actually attach trigger interrupts properly,
since the interrupts need to know which class instance methods to call. Since
`SetListArduino` really should be a singleton class, requiring the name you use
to be `SetListImage` fixes this issue.

## General comments on SetListArduino library structure

The SetListArduino library has two main classes: `class SetListArduino` and
`class SetListDevice`. The class `SetListArduino` is what you mainly have to 
worry about when coding an Arduino sketch; it orchestrates all of the 
communication between LabView and the controlled devices connected to your 
Arduino.

`SetListDevice` is a templated class which inherits from a generic class 
`SetListBase` (which does nothing by itself). When you register a device by
calling `SetListImage.registerDevice(...)`, `SetListArduino` will create an 
instance of `SetListDevice`. This instance of `SetListDevice` contains the 
SetList and pointers to any callbacks necessary to execute said SetList.

To make this more concrete, consider a DDS-type device we want to control (like
in the example sketch above). Say I have already instantiated the device itself, 
call it `DDS1`. To make it computer controllable, I first need to register it
with the singleton class object, `SetListImage`:

~~~~~~~~~~{.cpp}
SetListImage.registerDevice(DDS1, 0);
~~~~~~~~~~

`SetListArduino` has now added the object `DDS1` to position 0 in an array 
data member it owns called `_deviceList`. I can register more devices if I wish,
being sure to use sequentially increasing channel numbers so they are inserted
properly into the array `_deviceList`.

Now, when I register a command, I need to tell the Arduino what serial 
character it should listen to during SetList programming, and what it should
do when it receives such a command. To this effect, I use

~~~~~~~~~~{.cpp}
// registerCommand(const char * command, int channel, Callback function)
SetListImage.registerCommand("f", 0, setDDSFreq);

// somewhere else in your sketch...

void setDDSFreq(AD9954 * dds, int * params){
...
}
~~~~~~~~~~

The callback function, `setDDSFreq`, must take two arguments: a pointer of 
the same type as the device you're trying to control (here it is `AD9954`), and
an `int *` pointer to a list of parameters. When a particular SetList line is 
executed, `SetListDevice` will call that function and pass it a pointer to 
your device (ie, DDS) and a pointer to the parameter array.

### Serial programming sequence of events

Once you've uploaded your sketch with all the devices registered and command 
callbacks defined, etc., this is an example of what it expects to see from the
serial stream:

~~~~~~~~~~
@ 0
f 100000
f 150000
f 150000
f 500000
@ 1
r 100 200 500
f 50000
r 100 200 700
r 200 200 200
$
~~~~~~~~~~

What this means (and see the special short command summary in the next section)
is: 

* "Activate channel 0 for setlist programming"
* Programs 4 setlist lines with short command `f`, each taking a single parameter
* "Activate channel 1 for setlist programming"
* Programs 4 setlist lines; some have short command `r`, some short command `f`; short command `r` takes 3 arguments.
* Initialize for sequence to begin with command `$`.

Something worth noting: If you have, eg, 2 DDS devices on the same Arduino, you
don't need to create separate callback functions for the same behavior. As long 
as you register both, you can use the same callback. For example,

~~~~~~~~~~{.cpp}
SetListImage.registerCommand("f", 0, setDDSFreq);
SetListImage.registerCommand("f", 1, setDDSFreq);
SetListImage.registerCommand("f", 2, setPLLFreq);
~~~~~~~~~~

Here, there are two different DDSs which implement the same kind of behavior
(ie, "set a single-tone frequency"). They are on channels 0 and 1, so we
register that behavior with `SetListArduino`. There is also a PLL, which we 
might want to use the same short command for (eg, "f" for "frequency", to set
a single-tone output frequency). But, because it is a different device type, we
need a separate callback `setPLLFreq` which we would define elsewhere like

~~~~~~~~~~{.cpp}
void setPLLFreq(PLLDevice * pll, int * params){
    pll->setPLLFreq(params[0]);
}
~~~~~~~~~~

or something similar. Hopefully you're starting to get the gist.

## Special short commands

There are a few specially reserved short commands:

* `@` - activates a new device, eg, `@ 1` activates device on channel 1.
* `$` - initializes for new cycle. Call this when you want the Arduino ready to receive triggers from the PulseBlaster.
* `?` - echo setlist back to serial terminal.
* `#` - execute single setlist line, eg, `# 0 5` executes (index) line 5 on channel 0. Note this is the *index* of the line, not the line number -- the index counts from 0.

## Triggering requirements

Currently, as discussed briefly above, `SetListArduino` immediately outputs
the state for the first line after receiving the serial command `$` indicating
a sequence is beginning. It advances to the next line upon a falling-edge
trigger, and then switches the interrupt to listen for a falling- or 
rising-edge. 

When it encounters said trigger, an ISR routine jumps in and loops over the 
list of registered devices, calling `_deviceList[index]->executeSetList(_line)`
to execute the callbacks for the next line.

Because this is a single-threaded microcontroller, be cognizant of timing 
delays! In a spot check, it looks like a "simple" device which just toggles
the output of a digital pin on the Arduino, there was a 6.25 us delay between 
the trigger edge and the pin state change. Depending on how many devices you
have connected and how fast the Arduino can update each device, there may 
be a substantially longer delay before it actually updates an output.

For this reason, you also don't want triggers to come too soon after each other!
As always, test offline before stretching to the limit.

At any rate, LabView should provide a single state change trigger for each ramp
line in SetList where some controlled device needs to change its output. Thus,
as long as you aren't changing outputs too quickly, you should be fine. Just be
careful it is doing what you expect.

## Implementing Arduino I/O as a device

As alluded to above, you can actually use the digital/analog output pins on the
Arduino itself. Just register a device of type `int` (or some other dummy 
class). Then, in the callback function, change the output of your line as 
desired. You can get creative!

## Future Development

For better timing, we might consider implementing a feature where the Arduino
pre-programs a device (eg, AD9954 DDS) before the next SetList trigger. When
the trigger arrives, it simply has to trigger a (fast) digital IO update on the 
controlled device. Or, if we care even more about timing, we could implement
additional SlavedDigital channels in LabView to trigger the actual device IO 
update. At some level, however, it becomes necessary to use an FPGA for precise
timing applications.

## Sr Lab Arduino-controlled devices...

Currently, we have the Arduino controlling a few different devices in the Sr
lab. These libraries are also in the AMOArduino repository, and documentation
of the hardware should be on the [JQI wiki](https://jqi-wiki.physics.umd.edu).

* DDS boards
  * Output single-tone frequencies
  * Output linear frequency sweeps
  * More complex functionality can be implemented (see AD9954 library).
* ADF4350 PLLs
* ADF4107 PLLs, other Analog Devices PLL chips in use for OPLL Beatnote locking
* AD536x DACs
  * Library still needs some polishing; control 8- or 16-channel DAC chips from Analog Devices.
  * Again, could output single voltage or interpolated sweep. However, the intention here is to use these for analog channels that don't need updating during a cycle.


