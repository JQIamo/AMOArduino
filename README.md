# Arduino libraries

These are a few arduino libraries we are using in the UMD/JQI AMO group.

More info to come later, but...

## AD9954

This one is a class to handle SPI/serial communication with an AD9954 DDS chip.
Still in development, but currently supports single-frequency setpoints and bi-directional 
linear ramps, controlled by pin PS0.

## LockFreq

This class handles a voltage input from a frequency lock signal, and updates the corresponding
DDS frequency to track the lock.

## ADF4350

Class to handle SPI/serial communication with the ADF4350 PLL chips (slash eval boards...)

## MyEEPROM

Class to interface with an external EEPROM chip (specifically, 24LC16B). The Arduino Due should be able to utilize part of 
its onboard Flash memory through in-application programming on the SAM3X, but I wasn't able to get that to work. Ultimately, it
seemed easier to just use an external EEPROM chip.

## SimpleLCD

Wrapper class for writing to SparkFun's Serial LCD display: [https://www.sparkfun.com/products/9066](https://www.sparkfun.com/products/9066). NOTE: you want the 3.3V version for the Arduino Due!

## AD536x

Library for working with Analog devices' AD536x family DACs. Should work with { AD5360, AD5361 }  (16 channels, 16/14 bit DAC) and { AD5362, AD5363 } (8 channels, 14/16 bit DAC).
