This is work in progress.

# Pinhole camera

TLDR; DIY pinhole camera with mechanical shutter and film winder.

The idea is to build pinhole camera with mechanical shutter, one way film winder and light sensor. Future plans include bluetooth communication with android application to control various settings of camera.

### Disclimer

I have to say that I'm not a pro in any of these fields: electronics, arduino, c programming or pinhole design. Everything I've done here I did for the first time, thus I might be wrong and especially there might be better and more efficient ways of achiving same thing. In this document I will try to describe process of creating camera. Part list and software versions can be found at the end of page.

## Electronics

I started from the purchase of basic components: atmega, power regulator, leds, buttons, motors, breadboard and connectors. Steps in this chapter:

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

	avrdude -v -cbsd -pATmega168 -B16 -b9600 -i200

	-v - sets verbose mode
	-c bsd - selects type of programmer we are using
	-p ATmega8 - selects microcontroller we have connected
	-B 16 - sets clock speed
	-b9600 - sets lpt port speed
	-i200 - ISP clock delay

Expected end of output:

	Reading | ################################################## | 100% 0.05s

	avrdude: Device signature = 0x1e9406
	avrdude: safemode: lfuse reads as FF
	avrdude: safemode: hfuse reads as DD
	avrdude: safemode: efuse reads as 0

	avrdude: safemode: lfuse reads as FF
	avrdude: safemode: hfuse reads as DD
	avrdude: safemode: efuse reads as 0
	avrdude: safemode: Fuses OK

	avrdude done.  Thank you.

Things i learned using this programming chip: 

* board must be always powered - there is some voltage coming from lpt port and power led on breadbord might be on, but it't not enough voltage for atmega
* nothing can be connected to digital or analog pins - not even a led on pin 13, it will disrupt programmer and cause "invalid signature" error
* if you are getting errors try to increase logging verbosity by adding -vvv to above command
* if it still doesn't work double check connections

If everything goes well avrdude will print device signature and fuse values. Before we start burning process, arduino ide must be configured. Open ~/.arduino/preferences.txt file and change this option, this will give more details in case of any error:

	upload.verbose=true

Next open progremmers.txt, and change options to:

	parallel.protocol=bsd
	parallel.delay=200

Now start Arduino ide from terminal to see logs. In Tools menu select "Parallel programmer", select board "Arduino NG or older /w Atmega168" and finally "Burn bootloader". Wait few minutes. 

Microcontroller is now ready to be programmed. Disconnect ISP programmer, connect serial to usb adapter (RX, TX, power), and led diode to pin 13. In Arduino Ide load blink example (File>Examples>Basics>Blink). Press reset button on breadboard, and then (within few seconds) press upload button in ide. If led blinks - your controller is ready.

### Prototyping

![](https://raw.github.com/Eyjafjallajokull/atmega-pinhole/master/docs/source.jpg)

Having Atmega + Arduino IDE setup I experimented separately with basic components. Starting from buttons and diodes to motors. To play with buttons and diodes I used examples from arduino IDE, they are easy to understand and even documented. I only had problems with stepper motor which seems to incredibly slow by default.

```arduino
#include <Stepper.h>

int stepsPerRevolution = 64;
int rpmSpeed = 150;
int stepsPerMove = 512;
Stepper stepper(stepsPerRevolution, 5, 6, 7, 8);

void setup()
{
  stepper.setSpeed(rpmSpeed);
}

void loop()
{
  stepper.step(stepsPerMove);
  delay(1000);
}
```

I set `stepsPerRevolution` to 64 to match motor specification, so in theory `stepper.step(512)` should move motor 8 times (512/64), but it only turns 45 degrees. I tried setting various speeds (1-500) and 150 seems to be fastest value. I will leave this veird motor for now, it turns and that's what I need now.

Lets take a little step back and think about organizing code. Altough Arduino programming language supports creating classes and objects, I won't take advantage of it, bacause using those structures implies larger codebase. I'm using Atmega168 which has 14kb memory for code. I came across handy library called FiniteStateMachine - name describes everything. By using finite state machine pattern code will be easy to read and debug.

// @todo example usage, source

// @todo cmdmessenger

### Camera circuit and code

![](https://raw.github.com/Eyjafjallajokull/atmega-pinhole/master/docs/pcb.jpg)

After a lot of trial and error I designed whole circuit which is available in [fritzing](https://github.com/Eyjafjallajokull/atmega-pinhole/blob/master/design/pcb-design.fzz) or [png](https://raw.github.com/Eyjafjallajokull/atmega-pinhole/blob/master/design/pcb-design-generated/pcb-design-schematic.png) format. PCB designs are also available in repository.

## Pinhole and casing

![](https://raw.github.com/Eyjafjallajokull/atmega-pinhole/master/docs/casing.jpg)

## Notes:

Part list:

* Atmega168
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
