# LockFreq

A frequency lock wrapper class to the AD9954 DDS controller. Will update the DDS frequency to follow a voltage on one of the analog input pins.

## Getting started

You'll need to include the DDS and SPI classes, along with this one:

    #include <SPI.h>
    #include <AD9954.h>
    #include <LockFreq.h>
    
    AD9954 DDS(SS_Pin, Reset_pin, update_pin, PS0, PS1, OSK); // whatever pins you're using for these functions...

    // declare the Lock object.
    // POT_Pin: Analog input pin with the voltage you want to track.
    // "Resolution" and "Multiplier" are used to calibrate the sensitivity of the ADC voltage, along with the channel spacing.
    // The Arduino Due has a 12-bit ADC -> digital voltage values between 0 and 4095.
    // A resolution of 3, for example, means that the ADC must change by 3 for the LockFreq class to recognize a new setpoint.
    // the multiplier currently specifies a bitshift on the ADC value to calculate the frequency. So, for example, the Due reads
    // a digital voltage value of 300, and the multiplier is 6 -- then, the calculated frequency would be 300*2^6 = 19.2 MHz.
    // The channel spacing, then, is given by
    // channel spacing = (resolution)*2^(multiplier)
    // I'm thinking about changing this so the multiplier is literally a multiplicative factor (rather than bit-shift value),
    // although originally I did it this way thinking "bit shift must be faster than multiplication" -- we're using this
    // in a (admittedly slow) lock scheme, but I didn't want to do anything that might constrain the bandwidth.

    LockFreq Lock(POT_Pin, resolution, multiplier);

    void setup(){
        SPI.begin();
        SPI.setClockDivider(4);
        SPI.setDataMode(SPI_MODE0);

        Lock.initialize(DDS, baseFrequency, initialFrequency); // initialize the lock with a particular base frequency
                                                               // (meaning the voltage will shift from this base freq.)
                                                               // an initial frequency of 0 will force LockFreq to poll the
                                                               // POT for an initial voltage value. Otherwise, it will only poll
                                                               // the POT when you call Lock.updateFreq().

    }


    void loop(){
        Lock.updateFreq(); // update the frequency on the DDS by polling the POT for a new voltage.
    }


## Implemented features

In addition to the features shown above, you can also update the `baseFreq`:

    LockFreq::updateBaseFreq(unsigned long baseFreq); // updates the base frequency.

If you would instead like to specify a "center" frequency (ie, the frequency output when the
Arduino reads a digital voltage value of 2048, AKA 1.65V) so your LockFreq is symmetric about this frequency, you do so with

    LockFreq::updateCenterFreq(unsigned long centerFreq);

You can also get the current setpoint with

    LockFreq::getSetpoint(); // returns the current voltage setpoint. to get the current *frequency*,
                            // use DDS.getFreq();


    
