# Pinhole camera

This is work in progress.

TLDR; DIY pinhole camera with mechanical shutter and film winder.

### Disclimer

I have to say that I'm not a pro in any of these fields: electronics, arduino, c programming or pinhole design.
Everything I've done here I did for the first time, thus I might be wrong and especially there might be better and more efficient ways of achiving same thing. In this document I will try to describe process of creating camera. Part list and software versions can be found at the end of page.

## Electronics

I started from the purchase of basic components: atmega, power regulator, breadboard and connectors. Steps in this chapter:

* setup power supply and basic atmega circuit on breadboard
* create ISP programmer using lpt port
* burn bootloader
* build prototype circuit on breadboard and write Arduino code
* etch and solder PCB

### Setting up Atmega

![](https://raw.github.com/Eyjafjallajokull/atmega-pinhole/master/docs/breadboard.jpg)

First setup power input on breadboard. Then add atmega chip, reset button and clock. This is very well described on this page http://www.arduino.cc/en/Main/Standalone

Next step is to burn arduino bootloader on atmega controller. The best way to do this is by using AVR programmer like AVRISP mkII or USBtinyISP. Its also possible to do this using arduino board (many tutorials available). I didn't have any of those, but I found that it's possible to do this via parallel port.

Building parellel port ISP programmer is super easy, just take a look on schematic:

![](https://raw.github.com/Eyjafjallajokull/atmega-pinhole/master/docs/bsd-programmer.png)
*[source](http://avrprogrammers.com/programmers/Parallel_Port/avr-programmer-bsd) *

To poperly connect atmega to ISP programmer you will need to know controller [pinout](http://arduino.cc/en/Hacking/PinMapping).

Now test your parallel programmer:

	avrdude -vv -cbsd -pATmega168 -B16 -b9600 -i200

	-vv - sets verbose mode
	-c bsd - selects type of programmer we are using
	-p ATmega8 - selects microcontroller we have connected
	-B 16 - sets clock speed
	-b9600 - sets lpt port speed
	-i200 - ISP clock delay

Things i learned when programming chip: 

* board must be always powered - there is some voltage coming from lpt port and power led on breadbord will be on, but it't not enough for atmega
* nothing can be connected to digital or analog pins - not even a led on pin 13, it will disrupt programmer and cause "invalid signature" error.

If everything goes well avrdude will print device signature and fuse values. Before we start burning process, arduino ide must be configured. Open ~/.arduino/preferences.txt file and change this option, this will give more details in case of any error:

	upload.verbose=true

Next open progremmers.txt, and change options to:

	parallel.protocol=bsd
	parallel.delay=200

Now start Arduino ide from terminal to see logs. In Tools menu select "Parallel programmer", select board "Arduino NG or older /w Atmega168" and finally "Burn bootloader". Wait few minutes. 

Microcontroller is now ready to be programmed. Disconnect ISP programmer, connect serial to usb adapter (RX, TX, power), and led diode to pin 13. In Arduino Ide load blink example (File>Examples>Basics>Blink). Press reset button on breadboard, and then (within few seconds) press upload button in ide. If led blinks - your controller is ready.

### Prototyping

Having Atmega + Arduino IDE setup I experimented separately with basic components. Starting from buttons and diodes to motors. 

// @todo add some code snippets here

Altough Arduino programming language supports creating classes and objects, I won't take advantage of it, bacause using those structures implies larger codebase. I came across handy library called FiniteStateMachine - name describes everything.

// @todo example usage, source

### Camera circuit and code

![](https://raw.github.com/Eyjafjallajokull/atmega-pinhole/master/docs/pcb.jpg)
![](https://raw.github.com/Eyjafjallajokull/atmega-pinhole/master/docs/source.jpg)

After a lot of trial and error I designed whole circuit which is available in [fritzing](https://github.com/Eyjafjallajokull/atmega-pinhole/blob/master/design/pcb-design.fzz) or [png](https://raw.github.com/Eyjafjallajokull/atmega-pinhole/blob/master/design/pcb-design-generated/pcb-design-schematic.png) format. PCB designs are also available in repository.

## Pinhole and casing

![](https://raw.github.com/Eyjafjallajokull/atmega-pinhole/master/docs/casing.jpg)

## Notes:

Part list:

* Atmega8PU
* 28BYJ-48 stepper mottor
* ULN2003A stepper controller
* micro servo 3.7G motor
* L7805CV power regulator
* 16MHz crystal

Tools I'm using:

* Ubuntu 12
* avrdude 5.11.1
* Arduino ide 1.0.4
* Fritzing 0.8.3

Resources:

* atmega on breadboard
  * http://www.jameco.com/Jameco/workshop/JamecoBuilds/arduinocircuit.html
  * http://www.arduino.cc/en/Main/Standalone
  * http://avrprogrammers.com/programmers/Parallel_Port/avr-programmer-bsd
  * http://arduino.cc/en/Hacking/PinMapping
* stepper http://www.geeetech.com/wiki/index.php/Stepper_Motor_5V_4-Phase_5-Wire_%26_ULN2003_Driver_Board_for_Arduino
* servo http://arduino.cc/en/Reference/Servo
* temt6000 http://bildr.org/2011/06/temt6000_arduino/
* epprom http://www.protostack.com/blog/2011/01/reading-and-writing-atmega168-eeprom/
