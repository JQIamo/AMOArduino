/*
   SetListArduino.cpp - Arduino Integration for SetList computer control
   Created by Neal Pisenti, 2014
   JQI - Strontium - UMD
   
   SetList is a labview-based computer control suite in use at the JQI.
   Code and more info can be found on GitHub: 
   https://www.github.com/JQIamo/SetList

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



/* 
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
		* Library still needs some polishing; control 8- or 16-channel DAC chips
		  from Analog Devices.
		* Again, could output single voltage or interpolated sweep. However,
		  the intention here is to use these for analog channels that don't need
		  updating during a cycle.
		  
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
	
	* LabView implements the class Device > Arduino, which has variable number
	  of "channels," each of which can have a variable number of Columns
	  (regular or digital) configurable by the user.
	* One of these columns can specify a short command; eg, "r" for ramp or "f"
	  for frequency output. If only one type of behavior is allowed (eg, 
	  "voltage" output), then user can configure a default short command.
	* LabView's Arduino class 
	* The user, in setting up the device, now just has to negotiate command
	  mappings to controlled devices' setter functions. eg:
	  
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
	  
	  TODO: 
	  * Write library!
	  * Keep track of functions that a "controllable" Arduino device library
	    must implement to be compatible with SetListArduino to be useable... 
	    is there a way for the compiler to know this? I suppose it'll throw an
	    error if the function isn't there, so no worries.
	  * Document it!
*/


#include "Arduino.h"
#include "SetListArduino.h"



/***********************************************
    SetListBase Class definitions
************************************************/

// No need to do anything here, simply a container for SetListDevices.

SetListBase::SetListBase(){;}   
void SetListBase::executeSetList(int pos){;}
void SetListBase::insertToSetList(int pos, void(*function)(int *), int * params){;}
int SetListBase::test(){;}


/***********************************************
    SetListArduino Class Definitions
************************************************/

// SetListArduino() -- Constructor for SetListArduino class.
//      Initializes some private vairables, etc.

SetListArduino::SetListArduino(int triggerChannel) :
    _deviceCount(0),
    _serialTerm('\n'),
    _last(NULL),
    _line(0)
{
    _triggerChannel = triggerChannel;
    
    // configure some stuff for the serial parsing...
    strcpy(_delim, " ");        // set string delimiter for strtok_r
    strcpy(_activateCmd, "@");  // set "activate device" command char
    strcpy(_initRunCmd, "$");   // set "ready for triggers" command char
                                // resets _line, and readies for programmed run.
                                // Arduino should then listen for a falling edge
    
    clearSerialBuffer();
}



// registerCommand() -- adds a command to the list of "recognized" 
//      SetListArduino commands. This should be taken care of in the setup()
//      portion of our Arduino sketch.
//
// channel: which device channel to register the command with
// command: "short command" to be sent from SetList over serial.
// function: name of the function to use in the callback
//              This function must take a reference to a parameter list, eg,
//              void myCallbackFunc(int * params){ ... }
 
void SetListArduino::registerCommand(int channel, const char * command, void(*function)(int *)){
    // reallocate memory block to accomodate newly registered serial command;
    _commandList = (SerialCommandCallback *) realloc(_commandList, 
                        (_commandCount + 1)*sizeof(SerialCommandCallback));  
    
    // copy new command into _commandList
    strncpy(_commandList[_commandCount].command, command,
                        SERIALCOMMAND_MAXCOMMANDLENGTH);
    _commandList[_commandCount].channel = channel;
    _commandList[_commandCount].function = function;
    
    // increment count by 1
    _commandCount ++;
}


// firstTrigger - execute setlist callbacks for the first trigger
void SetListArduino::triggerUpdate(){
    for( int i = 0; i < _deviceCount; i++){
        _deviceList[i].executeSetList(_line); 
    }
    // increment line counter.
    _line++;
}


void SetListArduino::clearSetList(){
    _line = 0;
    _setlistLength = 0;
}

int SetListArduino::getTriggerChannel(){
    return _triggerChannel;
}


// readSerial() -- function to parse serial stream, look for commands, and 
//      add the appropriate callbacks to the appropriate device's _setlist.
// 
// This is based heavily off of kroimon's SerialCommand library:
//      https://github.com/kroimon/Arduino-SerialCommand

void SetListArduino::readSerial(){
    while (Serial.available() > 0){
        // read in next character from serial stream
        char inChar = Serial.read();
        
        if (inChar == _serialTerm) {
            // reached serial line terminator, so tokenize string
            char * command = strtok_r(_buffer, _delim, & _last);
            char * param;
            if (strncmp(command, _activateCmd, 
                    SERIALCOMMAND_MAXCOMMANDLENGTH) == 0){
                // LabView sent command to switch active devices
                _line = 0;
                _setlistLength = 0; // reset counter for setlist...
                                    // all of the logic here assumes you're
                                    // sending setlists of equal length for all
                                    // devices!!
                Serial.println("Place1");
                Serial.println(command);
                param = strtok_r(NULL, _delim, & _last);
                Serial.println(param);
                if (param != NULL) {
                    // i think atoi returns 0 for "non-numeric" string vals
                    int channelNum = atoi(param);
                    
                    if(channelNum > 0 && channelNum <= _deviceCount){
                        _activeDevice = channelNum;
                    } else {
                        if (ERROR_CHECK) {
                            Serial.println("ArduinoError1");
                        }
                    }
                } else {
                    // something is wrong
                    if (ERROR_CHECK) {
                        Serial.println("ArduinoError2");
                    }                    
                }      
            } else if (strncmp(command, _initRunCmd,
                            SERIALCOMMAND_MAXCOMMANDLENGTH) == 0){
                Serial.println("place2");
                _line = 0;
                //attachInterrupt(_triggerChannel,
                           // SetListISR::firstTriggerInterrupt, FALLING);
            } else if (command != NULL) {
                // Last case, if command isn't one of the "special" commands,
                // try to match it with list of registered commands.
                boolean matched = false;
                for (int i = 0; i < _commandCount; i++) {
                    Serial.print("in loop");
                    Serial.println(i);
                    Serial.print("command device: ");
                    
                    Serial.println(_commandList[i].command);
                    
                    // check to see if command is in command list,
                    // and matches with currently activated device.
                    // will want to think of a good way to report
                    // "no match found" to SetList/Labview.
                    if ((strncmp(command, _commandList[i].command, 
                            SERIALCOMMAND_MAXCOMMANDLENGTH) == 0) /*&&
                            (_commandList[i].channel == _activeDevice)*/){
                        Serial.println("h1");
                        int paramList[MAX_PARAM_NUM];
                        Serial.println("h2");
                        for (int p = 0; p < MAX_PARAM_NUM; p++){
                            Serial.println("h3");
                            paramList[p] = atoi(strtok_r(NULL, _delim, &_last));
                        }
                        Serial.println("h4");
                        //Serial.println(typeof(int));
                        Serial.println("ff");
                        Serial.println(_line);
                        int tt = _deviceList[_activeDevice  - 1].test();
                        Serial.println("shit shit shit");
                        Serial.println(tt);
                        Serial.println("shit");
                        (&(_deviceList[_activeDevice-1]))->insertToSetList(_line++,
                            _commandList[i].function, paramList);
                            Serial.println("h5");
                        matched = true;
                        if (ERROR_CHECK){
                            Serial.println("ok");
                        }
                        break;
                    }
                }   // end loop over _commandList
                
                // If command not matched, tell labview if ERR_CHECK defined.
                if (!matched && ERROR_CHECK) {
                    Serial.println("ArduinoError3");
                }
            } else {
                Serial.println("end");
            }
            
            clearSerialBuffer();    // clear out serial buffer
                                    // to prepare for next line.
        } 
        // If inChar isn't the serial line terminator, 
        // just add it to buffer and repeat.
        // if _bufPos > serial command buffer length, currently loose chars.
        // We will want to handle this properly at some point.
        else if(isprint(inChar)) {  // only add printable chars into buffer
            if (_bufPos < SERIALCOMMAND_BUFFER) {
                _buffer[_bufPos++] = inChar; // add char to buffer
                _buffer[_bufPos] = '\0';     // make sure null-terminated
            }
        }
    }   // end while(Serial.available());
}   // end readSerial();


void SetListArduino::clearSerialBuffer(){
    _bufPos = 0;
    _buffer[0] = '\0';
}



/***********************************************
    SetListISR Class Definitions
************************************************/

// see note in SetListArduino.h as to why we are doing this...

extern SetListArduino SetListImage;

void SetListISR::firstTriggerInterrupt(){
    SetListImage.triggerUpdate();
    
    // switch interrupt to trigger on CHANGE for rest of trigger pulses
    int triggerChannel = SetListImage.getTriggerChannel();
    //detachInterrupt(triggerChannel);
    //attachInterrupt(triggerChannel, SetListISR::restTriggerInterrupt, CHANGE);
}

void SetListISR::restTriggerInterrupt(){
    SetListImage.triggerUpdate();
}

