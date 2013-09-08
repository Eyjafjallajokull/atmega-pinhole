## UNDER HEAVY DEVELOPMENT

# Pinhole camera

## TLDR

DIY pinhole camera with mechanical shutter and film winder.

[some photos]

## Intro



### Disclimer

I have to say that I'm not a pro in any of these fields: electronics, arduino, c programming, android programming or pinhole design.
Everything I've done here I did for the first time, thus I might be wrong and especially there might be better and more efficient ways of achiving same thing.

## Features

* shutter will be opened and closed by tiny servo motor
* one way winding with stepper motor
* temt6000 light sensor
* whole thing is controlled by atmega chip
* pinhole itself must be easy to replace
* adapters for full frame picture or for cinema view

## Base electronic circuit

### Setting up Atmega

First setup power input on breadboard. Then add atmega chip, reset button and clock. 

[schemat]
[zdjęcie]

Before we can start programming we need to burn bootloader to atmega. The best way to do this is by using chip programmer like ???. I didn't have one but I found that it't possible to do this via parallel port. It works, however it takes some time to complete - about 

[schemat kabla]
[zdjęcie kabla]
[schemat połączenia]
[zdjęcie z arduino]

Things i learned when programming chip: 

* board must be always powered - there is some voltage coming from lpt port and power led on breadbord will be on, but it't not enough for atmega
* nothing can be connected to digital or analog pins - not even a led on pin 13, it will disrupt programmer and cause "invalid signature" error.

To test your programmer try command:

	avrdude -vv -cbsd -pATmega168 -B16 -b9600 -i200

	-vv - sets verbose mode
	-c bsd - selects type of programmer we are using
	-p ATmega8 - selects microcontroller we have connected
	-B 16 - sets clock speed
	-b9600 - sets lpt port speed
	-i200 ???

If everything goes well avrdude will print device signature, and fuse values. We are almost ready for some burning. Open arduino ide ~/.arduino/preferences.txt file and change this option, this will give more details in case of any error:

	upload.verbose=true

Next open progremmers.txt, and change options to:

	parallel.protocol=bsd
	parallel.delay=200

Now start Arduino ide from terminal to see logs. In Tools menu select "Parallel programmer", select board "Arduino NG or older /w Atmega168" and finally "Burn bootloader". Wait few minutes. 

Microcontroller is now ready to be programmed. Disconnect programmer, add serial to usb adapter, and led diode to pin 13. In Arduino Ide load blink example (File>Examples>Basics>Blink). Press reset button on breadboard, and then (within few seconds) press upload button in ide.



### Arduino

### Android

## Notes:

### Tools i'm using

* Ubuntu 12
* avrdude 5.11.1
* Arduino ide 1.0.4
* Fritzing 0.8.3

### Sources

* atmega on breadboard
  * http://www.jameco.com/Jameco/workshop/JamecoBuilds/arduinocircuit.html
* stepper 
