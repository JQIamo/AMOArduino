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

/*! \mainpage SetListArduino
	
	Doxygen documentation for the SetListArduino library.
	
	This library provides drop-in integration for Arduino-controlled devices
	to the LabView-based computer control suite in use at the JQI. More
	information can be found in the [README](md__r_e_a_d_m_e.html).
	
	Repository hosted at [GitHub](https://github.com/JQIamo).
	
	\sa [JQI GitHub Page](https://github.com/JQIamo)
*/


#include "Arduino.h"

#ifndef SetListArduino_h
#define SetListArduino_h 


// If you want to override these, do so at the top of your arduino sketch 
// before including <SetListArduino.h>

//! Number of lines allocated for SetList table.
#ifndef MAX_SETLIST_LINES
#define MAX_SETLIST_LINES 512	// defines max number of setlist lines
#endif

//! Length of SetListArduino::_deviceList; Maximum number of devices allowed.
#ifndef MAX_DEVICE_NUMBER
#define MAX_DEVICE_NUMBER 6
#endif

//! Length of parameter buffer; Maximum number of parameters allowed to be passed from LabView in a given command.
#ifndef MAX_PARAM_NUM
#define MAX_PARAM_NUM 8	        // Max number of parameters for given command
#endif                          // use to initialize command array buffer.

//! Serial command buffer length. Serial lines cannot be longer than this many characters.
#ifndef SERIALCOMMAND_BUFFER    // Buffer length for serial commands.
#define SERIALCOMMAND_BUFFER 512
#endif

//! Maximum length of short commands passed from LabView.
/*!
	\sa SetListArduino::registerCommand(), SetListArduino::readSerial()
*/
#ifndef SERIALCOMMAND_MAXCOMMANDLENGTH   // Max "short command" length
#define SERIALCOMMAND_MAXCOMMANDLENGTH 8
#endif

//! Boolean to enable/disable serial error checking.
/*!
	If true, SetListArduino will pass back error codes to LabView.
	
	\sa SetListArduino
*/
#ifndef SETLIST_ERROR_CHECK
#define SETLIST_ERROR_CHECK 1   // error check serial commands
#endif                          // pass "ok" to labview if ok, and some error
                                // string if not ok.

//! Boolean to enable/disable debug information.
/*!
	If true, SetListArduino will print back debug information to the serial 
	terminal.
*/
#ifndef SETLIST_DEBUG
#define SETLIST_DEBUG 1
#endif


// This establishes a generic callback function type.
// Since we don't know initially what device types there are, it casts
// the device to void*. Later, when the function is inserted into the particular
// SetListDevice's _setlist, we re-cast void* to the proper type.

//! Type-agnostic callback function for SetList devices.
/*!
	This defines a generic callback function type. Since we don't know a priori
	what device types are controllable by the Arduino, this casts
	the device type to `void *`. Later, when the callback function is inserted
	into SetListDevice::_setlist, we re-cast the `void *` pointer to the proper
	type.
	
	There is perhaps a more elegant way to do this, but the current structure 
	seems to work nicely.
*/
typedef void (*GenericSetListCallback)(void*, int*);


/***********************************************
    class SetListBase
************************************************/

// Base SetList class; sets up type inheritance.
// This is really just a wrapper for the more specific devices, instantiated
// using the template class below.

//! Parent container class for SetListDevice.
/*!
	SetListBase is just a container for SetListDevice; it does nothing by
	itself.
	\sa SetListDevice()
*/

class SetListBase {
    
    public:
        SetListBase();
        virtual void executeSetList(int pos);
        virtual void insertToSetList(int pos, void(*function)(void *, int *), 
                                            int * params);
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

//! Template class implementation of Setlist, for device-specific behavior.
/*!
	SetListDevice inherits from SetListBase. Each device connected to the
	Arduino will have its own instance of SetListDevice, which manages
	a list of callback functions and arguments to execute when the Arduino
	receives a trigger from the computer.
	
	\tparam Device
	\sa SetListBase(), 
*/
template <class Device> class SetListDevice : public SetListBase {

    public: 
    	
    	//! Type definition for device-specific callback function.
    	/*!
    		\sa SetListArduino::registerCommand()
    	*/
    	typedef  void (*SpecificSetListCallback)(Device *, int *) ;
      
        //! Constructor for SetListDevice.
        /*!	Creates an instance of SetListDevice.
        
        	Interesting point of info:
    		be careful about how you instantiate these objects! Calling, eg,
    		
    		    SetListDevice newDevice(DDS1);
    		
    		is different from calling
    		
    			SetListDevice newDevice = new SetListDevice(DDS1);
    		
    		The first will add `newDevice` to the stack, while the second method 
    		adds it to the heap. For our purposes, SetListDevice() is called 
    		from SetListArduino. We want `newDevice` to persist after that call 
    		passes  out of scope, so we should have the compiler allocate memory 
    		in the heap.
    	*/ 
        SetListDevice(Device & device)
        : _setlistLength(0)
        {
        	_device = & device;
			#ifdef SETLIST_DEBUG
				Serial.println("Creating new device");
			#endif        
        }
       
        
    	// insertToSetList -- appends a callback to the device's setlist.
    	//      pos: position in SetListDevice::_setlist to place callback
    	//      function: actual callback function
    	//      params: list of parameters to the callback function
    	
    	//! Inserts callback function into the _setlist of SetListDevice.
    	/*! \param pos Integer indexing the location to place the callback 
    		function in SetListDevice::_setlist.
    		
    		\param function GenericSetListCallback, which will be executed
    		when the Arduino receives the right trigger from the computer. It is
    		passed in as a GenericSetListCallback, but is re-cast as a 
    		SpecificSetListCallback using `reinterpret_cast`. It is done this
    		way to remain agnostic about the type of SetListDevice when it is
    		passed from SetListArduino. In your Arduino sketch, you should 
    		define this function and its behavior.
    		
    		\param params Array of parameters to pass to the callback 
    		function. Note, its length is limited by #MAX_PARAM_NUM

    		\sa SetListArduino::registerCommand
    	*/
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
    	
    	//! Returns the pointer value for a SpecificSetListCallback.
    	/*!
    		Used for debugging information.
    		
    		\param pos Integer index specifying the SetList line.
    		
    		\sa SetListArduino::readSerial()
    	*/
    	int  getSetListFunc(int pos){
    		return (int)_setlist[pos].function;
    	}
    	
    	//! Returns a pointer to the parameters to be passed to a SetList callback.
    	/*!
    		Used for debugging information.
    		
    		\param pos Integer index specifying the SetList line.
    		
    		\sa SetListArduino::readSerial()
    	*/
    	int * getSetListParams(int pos){
    		return _setlist[pos].params;
    	}
    	
    	//! Returns the length of a SetListDevice's currently programmed setlist.
    	int getSetListLength(){
    		return _setlistLength;
    	}
		
		//! Clears device's SetList table.
		/*! This function doesn't actually overwrite SetListDevice::_setlist.
			Rather, it resets the counter SetListDevice::_setlistLength to zero.
		*/
		void clearSetList(){
			_setlistLength = 0;
		}
    	
    	//! Executes callback function at a particular SetList line.	
    	/*! 
    		If #SETLIST_DEBUG is set, will print diagnostic information to 
    		serial terminal.
    		
    		\param pos Integer specifying the SetList line.
    	*/
    	void executeSetList(int pos){ 
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
	

		
	private:
	
		//! Pointer to the controlled device.
		/*!
			\sa SetListArduino::registerDevice()
		*/
		Device * _device;
	    
	    //! Data struct for storing setlist lines.
	    struct SetListCallback {
	        
	        //! Parameters passed to callback function
	        int params[MAX_PARAM_NUM];
			
			//! Callback function to be executed on this SetList line.	        
	        SpecificSetListCallback function;
	    };
	    
	    //! Empty SpecificSetListCallback function to be executed when no state change is needed on Device.
	    static void _holdValue(Device *, int * params){;}
	    
	    //! List of callback functions & params.
	    /*! 
	    	This member data contains the device's actual SetList. The length 
	    	is limited by #MAX_SETLIST_LINES.
	    	
	    	\sa SetListCallback, SetListArduino::registerCommand
	    */
	    SetListCallback _setlist[MAX_SETLIST_LINES];
	    
	    //! Length of device's SetList.
	    /*!
	    	Counter to keep track of how many SetList lines have been programmed
	    	to SetListDevice.
	    */
	    int _setlistLength;
	    
        
};



/***********************************************
    class SetListArduino
************************************************/

// This is the "useful" SetList class.
// It negotiates between individual SetListDevices and the computer control.

//! Primary "front-facing" class for the SetListArduino library.
/*!
	# Summary
	
	SetListArduino orchestrates the whole show. If you simply want to implement
	computer control for a new Arduino-controlled device, this is the only class
	you'll need to deal with.
	
	In an Arduino sketch, you will want to use the pre-declared singleton 
	instance, ::SetListImage. This ensures the Arduino handles trigger 
	interrupts properly.
	
	For specific information about implementing an Arduino sketch, look to the 
	[README](md__r_e_a_d_m_e.html).
*/

class SetListArduino {

    public:

		//! Constructor function.
		/*!
			Creates what (should be) a singleton instance of SetListArduino.
			
			\param triggerChannel Integer specifying where `attachInterrupt()`
			should bind. For Arduino Due, this will be the trigger channel pin
			number. For other Arduino boards, this might be the index of an ISR 
			which operates on a particular (pre-defined) pin. Check out the 
			[Arduino Documentation](http://arduino.cc/en/Reference/attachInterrupt)
			for details on what you should pass here.
		*/
		SetListArduino(int triggerChannel);
		
		// register a device with SetListArduino.
		// Pass the device in by reference, and specify the channel number
		// you set up in SetList on the computer.
		
		//! Registers an Arduino-controlled device with SetListArduino, indicating it should expect to receive SetList commands from the computer.
		/*!
			If #SETLIST_DEBUG is set, this will print debug information
			back to the serial terminal.
			
			**Be sure to read the `channel` details below!!** You'll be glad
			you did when you avoid a segfault.
			
			\param device Device, passed in by reference, which will
			be added to SetListArduino::_deviceList and enabled for computer
			control.
			
			\param channel Integer, **starting with zero**, corresponding
			to the index at which the device will be placed in the array
			SetListArduino::_deviceList. It is important that you
			define channels sequentially, beginning with 0, because
			SetListArduino will loop through `_deviceList[0] ... _deviceList[n]`
			where `n` is the number of registered devices. If it tries to
			execute SetList commands on a device that has not been registered,
			you risk a segfault. Ultimately this should be protected against,
			but for now just take note! Number your channels from `0...n` and 
			you'll be fine.
			
			For specific information on how this should be used in an
			Arduino sketch, look to the [README](md__r_e_a_d_m_e.html).
			
			\sa registerCommand(), SetListDevice
		*/
		template <typename Device> 
		void registerDevice(Device & device, int channel){
		    
		    // check that channel is in range...
		    // still have to be careful, because Arduino will segfault if
		    // you try to make calls to device channels which haven't been
		    // registered. See note above.
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
        
 

		//! Registers a callback command with SetListArduino, associated with a particular device channel.
		/*!
			This is how you set up a "connection table" between particular
			serial commands and the correct callback functions to control
			the state of a connected device. During `LoadHardwareImage`, LabView
			SetList will write a sequence of "short commands" and parameters to
			the serial terminal. SetListArduino listens for these, and depending
			on which device is being written to, associates those commands with
			a particular callback function and inserts the resulting callback 
			into that device's setlist using SetListDevice::insertToSetList().
			
			Information about how to implement this in an Arduino sketch can
			be found in the [README](md__r_e_a_d_m_e.html).
			
			\param command "Short command" SetListArduino will listen for on
			the serial terminal. This should match up with the short-command 
			set up in the LabView device.
			
			\param channel Integer specifying the device channel to register
			the command with. See details in SetListArduino::registerDevice().
			
			\param function Callback function to be executed.
		*/
        template <typename Callback>
        void registerCommand(
        	const char * command, 
        	int channel, 
        	Callback function)
        {
    		// reallocate memory block to accomodate newly registered 
    		// serial command;
    		_commandList = (SerialCommandCallback *) realloc(_commandList, 
                        (_commandCount + 1)*sizeof(SerialCommandCallback));  
    
    		// copy into _commandList
    		strncpy(_commandList[_commandCount].command, command,
                        SERIALCOMMAND_MAXCOMMANDLENGTH);
    		_commandList[_commandCount].channel = channel;
    		_commandList[_commandCount].function = 
    					reinterpret_cast<GenericSetListCallback>(function);
    		
    		_commandCount++; // increment count by 1
		};
        
        //! Clears the currently programmed setlist by resetting SetListArduino::_setlistLength counter.
        void clearSetList();
        
        //! Returns SetListArduino::_triggerChannel.
        /*!
        	\sa SetListArduino()
        */
        int getTriggerChannel();
        
        //! Forces update on all devices to next SetList line.
        /*!
        	This function loops through the registered devices, and calls
        	SetListDevice::executeSetList() on the active line.
        	
    		The active setlist line is kept track of by SetListArduino::_line.
        	
        	Make sure you have read the warning in 
        	SetListArduino::registerDevice(), because there is a risk of 
        	segfault if SetListArduino tries to execute callbacks for
        	non-existant devices.
        	
        	\sa registerDevice(), SetListDevice::executeSetList()
        */
        void triggerUpdate();   // trigger update to next line of setlist.
                                // the current line is kept track of with the
                                // _line state variable.
        
        
        // Serial parsing functions
        
        //! Listens to the serial stream for new commands from the computer.
        /*!
        	This should be called repeatedly in the `void loop(){...}` portion
        	of your Arduino sketch. For more information, look at the 
        	[README](md__r_e_a_d_m_e.html).
        */
        void readSerial();
        
        //! Resets the serial buffer after encountering a line termination character.
        void clearSerialBuffer();

        
    private:
    
    	//! State variable tracking active SetList line.
        volatile int _line;	    // next setlist line. Volatile since it'll be 
		                        // updated from an attachInterrupt routine.
		
		//! Member data containing the length of the programmed SetList.
		/*!
			Behavior of SetListArduino relies on the SetLists programmed from 
			LabView be of equal length. Specifically, 
			SetListArduino::_setlistLength is reset each time LabView activates
			a new device from the serial terminal, and is incremented for each 
			new SetList line passed for that device. There is rudimentary 
			error checking done to ensure each SetListDevice has a setlist of
			the expected length. Setting #SETLIST_ERROR_CHECK will cause
			SetListArduino to print back to the serial terminal indicating
			either the command was OK or whether there is an error.
		*/
		int _setlistLength;    	// Length of current setlist (# of lines);
		                        // this better be less than MAX_SETLIST_LINES!


		//! Binding for `attachInterrupt()` ISR routine.
		/*!
			For Arduino Due, this is the pin where you have attached
			the trigger channel. For other Arduinos, this is the ISR to use, 
			which works on a pre-defined pin.
			
			Interrupts are handled in the static class SetListISR.
			
			\sa SetListISR, SetListArduino()
		*/
        int _triggerChannel;	// For Arduino Due, this is the interrupt pin
        						// that the ISR will be attached to.
        						// For certain other arduinos, this should be  
        						// the ISR number; see documentation for 
        						// attachInterrupt() for more info.
        						// SetListArduino calls something like
        			// attachInterrupt(_triggerChannel, (*ISRFunc)(), CHANGE);
        
        
        // private variables for serial parsing
        
        
        
        // This struct maps serial command -> device channel & callback func
        
        //! Data `struct` containing a map between serial commands and SetList callback functions.
        /*!
        	Note, there is a maximum length to the serial "short command" set by
        	#SERIALCOMMAND_MAXCOMMANDLENGTH. See SetListArduino::readSerial()
        	for details, or the [README](md__r_e_a_d_m_e.html).
        	
        	\sa readSerial(), registerCommand(), registerDevice()
        */
        struct SerialCommandCallback {
            
            //! "Short command" to listen for on serial stream.
            char command[SERIALCOMMAND_MAXCOMMANDLENGTH + 1];
            
            //! Integer specifying which device channel should be associated with the given command.
            int channel;
            
            //! GenericSetListCallback function.
            /*! This is a type-agnostic recast of the SetList callback passed in 
            from SetListArduino::registerCommand().
            */
            GenericSetListCallback function;
        };
        
        //! Dynamically allocated list of registered commands.
        /*!
        	List of SetListArduino::SerialCommandCallback `structs`, which
        	SetListArduino::readSerial() uses to lookup incoming serial commands
        	when programming a new SetList.
        	
        	\sa readSerial(), SerialCommandCallback
        */
        SerialCommandCallback * _commandList;
        
        //! Serial stream line terminator.
        /*!
        	Set to "\n" in SetListArduino()
        */
        char _serialTerm;       // Line termination character. Defaults to "\n".
        
        
        //! Serial command buffer.
        /*! 
        	readSerial() stores characters here while looking for serial
        	termination character.
        */
        char _buffer[SERIALCOMMAND_BUFFER + 1];    	// Buffer for serial chars
                                                    // while waiting for term.
        
        //! Current position in the serial buffer
        int _bufPos;        	// Current position in the serial buffer
        
        //! Text delimiter to use for strtok_r in readSerial().
        char _delim[2];     	// Text delimiter; defaults to " "
        
        //! State variable used by strtok_r in readSerial().
        char * _last;       	// state variable used by strtok_r  
                            	// during text processing.
        
        //! Holds total number of registered commands.
        int _commandCount;  	// keeps track of registered command total
        
        //! Variable tracking the channel of "active" device.
        /*!
        	Here, "active" means SetList commands written to the serial
        	terminal will be added to that device's SetList. Devices are
        	activated with the special short command given in
        	SetListArduino::_activateDeviceCmd.
        */
        int _activeDevice;      // channel of "active" device
        
        
        //! Special short command to activate a particular device.
        /*! 
        	Sending "@ 0" to the serial terminal will activate the device
        	on channel 0. This short command expects a single parameter; any
        	additional parameters will be ignored.
        	
        	\sa SetListArduino::_activeDevice, readSerial()
        */
        static const char _activateDeviceCmd = '@';   

        //! Special short command to initialize a run.
        /*! 
        	Sending "$" to the serial terminal will place the Arduino in a state
        	where it is ready to receive triggers on 
        	SetListArduino::_triggerChannel.
        	
        	\sa readSerial()
        */
        static const char _initRunCmd = '$';    
        
        //! Special short command to echo each device's SetList back to the serial terminal.
        /*!
        	Sending "?" to the serial terminal will cause the Arduino to write
        	each device's setlist to the serial terminal, including information
        	about how many SetList triggers that device expects.
        	
        	\sa readSerial()
        */
        static const char _echoSetListCmd = '?';
        
  
        //! Special short command to execute a single setlist line on a particular channel.
        /*!
        	Sending "# (channel) (line)" will cause the Arduino to execute the
        	SpecificSetListCallback function on the given line and channel.
        	
        	For example, "# 0 5" would execute the SetList command line 5 for 
        	device 0.
        	
        	\sa readSerial()
        */
        static const char _executeSingleLine = '#';	


        // private variables for actually implementing the setlist
        //! Tracks number of registered devices.
        /*!
        	Variable keeping count of how many Arduino-controlled devices are
        	registered with SetListArduino for computer control.
        	
        	During the execution of a SetList, SetListArduino will loop through
        	each device in SetListArduino::_deviceList, up to the index given by
        	_deviceCount. See the warning in registerDevice() about avoiding
        	segfaults.
        	
        	\sa SetListArduino::_deviceList, registerDevice()
        */
        int _deviceCount;
        
        //! List of pointers to the registered devices.
        /*!
        	This is a list of pointers to devices you have registered using
        	registerDevice(). Be sure to look at the warning in registerDevice()
        	to avoid segfaults, because the channel you register with a given 
        	device is just the index in _deviceList.
        	
        	I concede this could be handled better, perhaps in a future release.
        */
        SetListBase * _deviceList[MAX_DEVICE_NUMBER];
        
        //! Boolean flag indicating whether an error has occurred.
        /*!
        	During readSerial(), if SetListArduino encounters an error such as
        	an out-of-bound index, non-rectangular setlist, or invalid command,
        	it will set `_errorFlag = true` and report back to LabView on the
        	serial stream.
        	
        	This feature can be enabled/disabled by setting #SETLIST_ERROR_CHECK
        */
        bool _errorFlag;


};




//! Static class which handles trigger interrupt routines.
/*! 
	# Summary/Rationale
	
	This is a bit wacky-hacky, but it's the best idea I could think of.
	The problem here is that I want to hide the attachInterrupt functions
	inside the SetListArduino class, but the attachInterrupt callback 
	cannot take any variables. Sadly, class methods are implicitly passed
	the class itself, ie, void firstTriggerInterrupt(SetListArduino::). 
	The hack we're using here is to create a static class SetListISR, which
	will call the SetListArduino class methods as appropriate. attachInterrupt
	will register callbacks in SetListISR, thus avoiding the above issue.

	The SetListArduino class is a singleton (ie, only one instance should
	ever be declared in an arduino sketch). This hack takes advantage of that,
	in that the SetListISR methods need to know which instance of SetListArduino
	to call the trigger interrupt methods on. In order for everything to work, 
	we had to pre-decide on what the singleton SetListArduino instance should be
	called; Dan and I decided SetListImage made the most sense. In the .cpp
	file, you'll see the declaration "extern SetListArduino SetListImage;"
	Then, these SetListISR static methods will call functions on the object
	SetListImage. All works as expected, but in the arduino sketch you'll be
	forced to use the name SetListImage, which is hopefully a *very* minor
	inconvenience!
	
	\sa ::SetListImage
*/
class SetListISR {
    public:
        
        //! ISR for "first trigger" of a SetList
        /*!
        	This ISR is attached to SetListArduino::_triggerChannel on a falling
        	edge trigger. Once SetListArduino sees that first falling edge, it
        	changes the ISR to trigger on `CHANGE` (ie, rising *or* falling 
        	edge).
        	
        	It is implemented this way because depending on whether SetList has
        	an even or odd number of lines, the PulseBlaster trigger channel
        	might be in a high or a low state, thus causing an off-by-one index
        	error in the SetList table.
        	
        	Note this also means that the first SetList line should be a
        	"static" output for all Arduino-controlled devices, ie, not a
        	DDS ramp or similar. See the [README](md__r_e_a_d_m_e.html) for 
        	more details.
        	
        	\sa SetListArduino::_triggerChannel, SetListArduino::SetListArduino()
        */
        static void firstTriggerInterrupt();
        
        //! ISR for all other SetList triggers after the first trigger.
        /*!
        	This ISR is attached to SetListArduino::_triggerChannel on a 
        	`CHANGE` event. See firstTriggerInterrupt() for more details, 
        	as well as the [README](md__r_e_a_d_m_e.html).
        	
        	\sa firstTriggerInterrupt(), SetListArduino::_triggerChannel, SetListArduino::SetListArduino()
        */
        static void restTriggerInterrupt();
};


#endif // SetListArduino_h