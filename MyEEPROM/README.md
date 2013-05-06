# MyEEPROM

A class to manage TWI communication between the Arduino Due and the 24LC16B EEPROM chip. 
(Might work for other EEPROM chips too, but mileage will vary).

## Getting started

Check out the examples folder, yo!

Basically, the 24LC16B has 8 blocks of 256 bytes each (for a total of 2 Kilobytes).
Usually, these I2C devices have a way to dynamically address each slave, but for a variety of reasons the 24LC16B
has a static address. Thus, you'll only be able to hang one of these bad boys off a given TWI.

You can address the EEPROM memory up to 16 bytes at a time. You want to use:

    MyEEPROM.write(int memAddress, const byte * data, byte length);

Here, `memAddress` is the memory address you're writing to. It should be a 3-digit hex value, like `0x100` -- the first digit specifies
which block you're addressing (therefore, should be limited from `0-7`). The next two digits specify the address of the first
byte you want to write. Thus, a `memAddress` of `0x120` would be addressing block 1, address 20.

Then, just pass in your data array, and the number of bytes you're writing (`length`). It will cut you off at 16 bytes if you try to 
write more than that at a time -- this is done to preserve the integrity of the data you *did* write; the EEPROM can only handle up to
one page at a time (which I'm lead to believe is 16 bytes) -- doing more will cause the memory address pointer to loop around and overwrite
the data you wrote at the beginning of this write cycle.

To read the data back out, simple use

    MyEEPROM.read(int memAddress, byte * buffer, byte length);

Ie, give it an address to start from, a buffer to read the data into, and a length (in bytes) you want to read.

Hopefully the examples should clarify this useage. Don't forget to initialize the Arduino Wire communication line first!

    #include <Wire.h>
    #include <MyEEPROM.h>
    
    void setup(){
        Wire.begin();
        byte data[] = {1, 2, 3};
        
        MyEEPROM.write(0x000, data, 3);
        // etc.
        
    }
