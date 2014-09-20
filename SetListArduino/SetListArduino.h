/*
   SetListArduino.h - Arduino Integration for SetList computer control
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


	Example sketch behavior:
	
	#include <SetListArduino.h>
	
	SetListArduino SetList(triggerChannel); // instantiate setlist object
	
	AD9954 DDS1(...);
	
	
	
	
	union {
	    DDS dds;
	    PLL pll;
	    } obj;
	    
	    obj.d
	
	void setup(){
		SetListImage.registerDevice(DDS1, 0);
		SetListImage.registerDevice(DDS2, 1); // etc.; (Device, channelNum)
		SetListImage.registerDevice(PLL1, 2);
		
		SetList.registerCommand("f", 0, &setFreq, 1); // command "f"
						// for device on channel 0, will call DDS1.setFreq(...)
						// with a single argument.
		// etc.
		
		
	
	
	}
	
	void setFreq(int frequency){...}
	
	void loop(){
		SetList.readSerial();	// listen for & process serial commands 
								// from computer
	}

void fcmd(){

}



*/

/* NOTES!!

	Ok, some things that need doing:
	
	Arduino-controllable devices should inherit from a parent "ControlledDevice"
	class. to still use libraries that we didn't write, figure out an easy way
	to encapsulate said library into this type. That way, can have an array
	of "ArduinoControlledDevices".
	
	Ugh, actually, that'll still be an issue if we want compiler errors when
	a given function is not defined.

*/


#ifndef SetListArduino_h
#define SetListArduino_h 


// If you want to override these, do so at the top of your arduino sketch 
// before including <SetListArduino.h>

#ifndef MAX_SETLIST_LINES
#define MAX_SETLIST_LINES 512	// defines max number of setlist lines
#endif
#ifndef MAX_DEVICE_NUMBER
#define MAX_DEVICE_NUMBER 6
#endif
#ifndef MAX_PARAM_NUM
#define MAX_PARAM_NUM 8	        // Max number of parameters for given command
#endif                          // use to initialize command array buffer.
#ifndef SERIALCOMMAND_BUFFER    // Buffer length for serial commands.
#define SERIALCOMMAND_BUFFER 512
#endif
#ifndef SERIALCOMMAND_MAXCOMMANDLENGTH   // Max "short command" length
#define SERIALCOMMAND_MAXCOMMANDLENGTH 8
#endif
#ifndef ERROR_CHECK
#define ERROR_CHECK 1           // error check serial commands.
#endif                          // pass "ok" to labview if ok, and some error
                                // string if not ok.
#ifndef SETLIST_DEBUG
#define SETLIST_DEBUG 1
#endif


// This establishes a generic callback function type.
// Since we don't know initially what device types there are, it casts
// the device to void*. Later, when the function is inserted into the particular
// SetListDevice's _setlist, we re-cast void* to the proper type.
typedef void (*GenericSetListCallback)(void*, int*);


/***********************************************
    class SetListBase
************************************************/

// Base SetList class; sets up type inheritance.
// This is really just a wrapper for the more specific devices, instantiated
// using the template class below.
class SetListBase {
    
    public:
        SetListBase();
        virtual void executeSetList(int pos);
        virtual void insertToSetList(int pos, void(*function)(void *, int *), 
                                            int * params);
        virtual int test();
        int publicVal;
        
        virtual int  getSetListFunc(int pos);
        virtual int * getSetListParams(int pos);
        virtual int getSetListLength();
        virtual void clearSetList();
        
};        



/***********************************************
    class SetListDevice
************************************************/

// Template for "device" class, where the type is specific 
// to each controlled device.
//
// Each SetListDevice owns its own setlist table, which is a sequence of
// callback functions. The object SetListArduino contains a list of pointers
// to SetListDevices, and upon receiving a trigger, tells them each to execute 
// their next callback function.
//
// Implementation of this class is in header file, because you apparently can't
// separate template class definitions into separate files...?
// But, we don't really care :P
template <class Device> 
class SetListDevice : public SetListBase {

    public: 
    	typedef  void (*SpecificSetListCallback)(Device *, int *) ;
      
        // Constructor. Pass in a device by reference.
        SetListDevice(Device & device)
        : _setlistLength(0)
        {
        	_device = & device;
			#ifdef SETLIST_DEBUG
				Serial.println("Creating new device");
			#endif        
        }
       
        int test(){
        	Serial.println("This should be 10. It is indeed "); 
            int crap = 10;
        	return crap;
        } 
        
        
    	// insertToSetList -- appends a callback to the device's setlist.
    	//      pos: position in _setlist to place callback
    	//      function: actual callback function
    	//      params: list of parameters to the callback function
    	
    	void insertToSetList(int pos, GenericSetListCallback function, int * params){
    	    
    	    // check that line you are appending is different from previous
    	    bool eq = false;
    	    if ((pos != 0) && (_setlist[pos - 1].function == 
    	    			reinterpret_cast<SpecificSetListCallback>(function))){
    	        eq = true;
    	        
    	        // now check if params are equal..
    	        for(int i = 0; i < MAX_PARAM_NUM; i++){
    	            if(_setlist[pos - 1].params[i] != params[i]){
    	                eq = false;
    	                break;
    	            }
    	        }
    	    }
    		if (!eq) {
    		    
    		   
    		    //cbtype * ff = static_cast <cbtype*>(function);
    		    _setlist[pos].function = reinterpret_cast 
    		    		<SpecificSetListCallback>(function);
    		} else {
    		    _setlist[pos].function = _holdValue;
    		}
    		// copy param list into SetListCallback...
    		memcpy(_setlist[pos].params, params, sizeof(params)*MAX_PARAM_NUM);
    		// increment setlistLength by 1
    		
    		_setlistLength++;
    	}
    	
    	int  getSetListFunc(int pos){
    		return (int)_setlist[pos].function;
    	}
    	
    	int * getSetListParams(int pos){
    		return _setlist[pos].params;
    	}
    	
    	int getSetListLength(){
    		return _setlistLength;
    	}
    		
    	void executeSetList(int pos){
    	    //int * params = _setlist[pos].params;
    	    
    	    // make sure you aren't calling a line that is out of bounds!
    	    if (pos < _setlistLength) {
    	    	(* _setlist[pos].function)(_device, _setlist[pos].params);
    	    } else {
    	    	#ifdef SETLIST_DEBUG
    	    		Serial.print("Line out of range. This device only has ");
    	    		Serial.print(_setlistLength);
    	    		Serial.println(" setlist lines.");
    	    	#endif
    	    }
    	}
	
		// clearSetList: clears current setlist table
		void clearSetList(){
			_setlistLength = 0;
		}
		
	private:
	
		// pointer to the actual device
		Device * _device;
	    
	    // Setlist callback; struct for storing setlist lines.
	    struct SetListCallback {
	        int params[MAX_PARAM_NUM];
	        void (*function)(Device *, int *);
	    };
	    
	    // the "don't update anything" function.
	    static void _holdValue(Device *, int * params){;}
	    
	    // the setlist... a list of pointers to callback functions & params
	    SetListCallback _setlist[MAX_SETLIST_LINES];
	    
	    int _setlistLength;
	    
        
};



/***********************************************
    class SetListArduino
************************************************/

// This is the "useful" SetList class.
// It negotiates between individual SetListDevices and the computer control.
class SetListArduino {

    public:
        // Constructor.
		// triggerChannel: integer specifying arduino digital channel to listen
		// 		for triggers from SetList.
		
		SetListArduino(int triggerChannel);
		
		// register a device with SetListArduino.
		// Pass the device in by reference, and specify the channel number
		// you set up in SetList on the computer.
		template <typename Device> 
		void registerDevice(Device & device, int channel){
		    
		    // check that channel is in range...
		    // still have to be careful, because Arduino will segfault if
		    // you try to make calls to device channels which haven't been
		    // registered.
		    if (channel >= 0 && channel < MAX_DEVICE_NUMBER){	    
				#ifdef SETLIST_DEBUG
					Serial.println("Initializing new device...");
				#endif
			
			// create new SetListDevice from the device you passed.
			// Fun fact I learned: pay attention to whether this is created
			// on the stack or the heap! Here, because we want the SetListDevice
			// to stick around, it should be allocated a memory block in the
			// heap, not just placed on the stack, which is what would happen if
			// you did "SetListDevice<Device> newDevice()".
			
				SetListDevice<typeof(device)> * newDevice = new SetListDevice<typeof(device)>(device);	
			
			
				_deviceList[channel] = newDevice;
				_deviceCount++;
			
				#ifdef SETLIST_DEBUG
					Serial.print("Device created on channel ");
					Serial.println(channel);
					Serial.print("Device total: ");
					Serial.println(_deviceCount);
				#endif
			} else {
				#ifdef SETLIST_DEBUG
					Serial.print("Invalid channel: ");
					Serial.println(channel);
				#endif
			}
		};
        
        
        //
      
 
//void SetListArduino::registerCommand(int channel, const char * command, void(*function)(void *, int *))
        
          
        // registerCommand() -- adds a command to the list of "recognized" 
		// SetListArduino commands. This should be taken care of in the setup()
		// portion of our Arduino sketch.

		// channel: which device channel to register the command with
		// command: "short command" to be sent from SetList over serial.
		// function: name of the function to use in the callback
		// 			This function must take a reference to a parameter list, eg,
//              	void myCallbackFunc(int * params){ ... }
        //template <typename Device>
        //void registerCommand(int channel, const char * command, void(*function)(Device *, int *)){
        template <typename CB>
        void registerCommand(int channel, const char * command, CB function){
    // reallocate memory block to accomodate newly registered serial command;
    _commandList = (SerialCommandCallback *) realloc(_commandList, 
                        (_commandCount + 1)*sizeof(SerialCommandCallback));  
    
    // copy new command into _commandList
    strncpy(_commandList[_commandCount].command, command,
                        SERIALCOMMAND_MAXCOMMANDLENGTH);
    
    _commandList[_commandCount].channel = channel;
    //_commandList[_commandCount].function = reinterpret_cast<mf>(function);
    _commandList[_commandCount].function = reinterpret_cast<GenericSetListCallback>(function);
    _commandCount ++; // increment count by 1
}
        
        
        
        
        
        void clearSetList();
        int getTriggerChannel();
        
        void triggerUpdate();   // trigger update to next line of setlist.
                                // the current line is kept track of with the
                                // _line state variable.
        
        
        // Serial parsing functions
        void readSerial();
        void clearSerialBuffer();

        
    private:
    
        volatile int _line;	    // next setlist line. Volatile since it'll be 
		                        // updated from an attachInterrupt routine.
		
		int _setlistLength;    	// Length of current setlist (# of lines);
		                        // this better be less than MAX_SETLIST_LINES!

        int _triggerChannel;	// For Arduino Due, this is the interrupt pin
        						// that the ISR will be attached to.
        						// For certain other arduinos, this should be  
        						// the ISR number; see documentation for 
        						// attachInterrupt() for more info.
        						// SetListArduino calls something like
        			// attachInterrupt(_triggerChannel, (*ISRFunc)(), CHANGE);
        
        
        // private variables for serial parsing
        
        // This struct maps serial command -> device channel & callback func
        struct SerialCommandCallback {
            char command[SERIALCOMMAND_MAXCOMMANDLENGTH + 1];
            int channel;
            //mf function;
            GenericSetListCallback function;
            //good; void (*function)(void *, int *);
            //void* function;
        };
        
        SerialCommandCallback * _commandList;
        
        char _serialTerm;       // Line termination character. Defaults to "\n".
        
        char _buffer[SERIALCOMMAND_BUFFER + 1];    	// Buffer for serial chars
                                                    // while waiting for term.
        int _bufPos;        	// Current position in the serial buffer
        char _delim[2];     	// Text delimiter; defaults to " "
        char * _last;       	// state variable used by strtok_r  
                            	// during text processing.
        
        int _commandCount;  	// keeps track of registered command total
        int _activeDevice;      // channel of "active" device
        
        // command to "activate" particular channel for writing setlist
        // expects a single parameter; eg, "@ 0" or "@ 1" to activate channel
        // 0 and 1, respectively.
        static const char _activateDeviceCmd = '@';   

        // command to initialize a run; prepares arduino to receive triggers.
        static const char _initRunCmd = '$';    
        
        // command to echo back setlist to serial terminal.
        static const char _echoSetListCmd = '?';
        
        // command to execute a particular setlist line.
        // expects 2 parameters, eg, "# 0 3" would execute line 3 on channel 0.
        static const char _executeSingleLine = '#';	


        // private variables for actually implementing the setlist
        int _deviceCount;
        SetListBase * _deviceList[MAX_DEVICE_NUMBER];
        bool _errorFlag;


};


// This is a bit wacky-hacky, but it's the best idea I could think of.
// The problem here is that I want to hide the attachInterrupt functions
// inside the SetListArduino class, but the attachInterrupt callback 
// cannot take any variables. Sadly, class methods are implicitly passed
// the class itself, ie, void firstTriggerInterrupt(SetListArduino::). 
// The hack we're using here is to create a static class SetListISR, which
// will call the SetListArduino class methods as appropriate. attachInterrupt
// will register callbacks in SetListISR, thus avoiding the above issue.

// The SetListArduino class is a singleton (ie, only one instance should
// ever be declared in an arduino sketch). This hack takes advantage of that,
// in that the SetListISR methods need to know which instance of SetListArduino
// to call the trigger interrupt methods on. In order for everything to work, 
// we had to pre-decide on what the singleton SetListArduino instance should be
// called; Dan and I decided SetListImage made the most sense. Thus, in the .cpp
// file, you'll see the declaration "extern SetListArduino SetListImage;"
// Then, these SetListISR static methods will call functions on the object
// SetListImage. All works as expected, but in the arduino sketch you'll be
// forced to use the name SetListImage, which is hopefully a *very* minor
// inconvenience!

class SetListISR {
    public:
        
        // split out interrupt routines based on "first trigger" and the
        // "rest of the triggers". This way the first trigger can be falling 
        // edge, while the rest can be on change.
        static void firstTriggerInterrupt();
        static void restTriggerInterrupt();
};


#endif // SetListArduino_h