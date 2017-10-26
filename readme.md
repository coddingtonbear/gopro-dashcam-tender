
![](https://s3-us-west-2.amazonaws.com/coddingtonbear-public/github/gopro-dashcam-tender/gdt.JPG)

I use my GoPro as a dashcam, and although I rarely need to take any footage
off of my device, the GoPro will slowly accumulate videos over time.  This
device provides an easy way for me to clear the GoPro at the end of the 
day by just plugging my GoPro into it as its charger.

## Components Required

In order of decreasing expense:

* 1x ESP-12F: These can be obtained for roughly $2 on AliExpress.  Other similar packages of the ESP8266 wifi module, or _maybe_ even the ESP-01 module should work with minimal modifications to the layout and schematic.
* 1x CH340G: ~$0.50 on AliExpress.
* 1x SOT-223 3.3V Voltage Regulator
* 2x SOT-23 NPN Transistors for the UART->ESP8266 reset circuit.  Note that there are alternative reset circuits.  See below for details.
* 1x HC-49 12MHz crystal
* Miscellaneous passives.

See the schematic's netlist for more details.

## Procedure

Essentially, what this device does is:

* Wait for a GoPro to be plugged-in to the USB-A port.  This can be detected because of the way connected USB devices pull up the D+/D- lines to indicate their operating speed.
* Connect to my home WIFI network to get an up-to-date UTC time using NTP.  Immediately disconnecting from my home WIFI network.
* Connect to the GoPro WIFI network, then:
  * Set the GoPro's time.
  * Clear all stored media.
  * Turn off the GoPro.

## Schematic

![](https://s3-us-west-2.amazonaws.com/coddingtonbear-public/github/gopro-dashcam-tender/gdt.svg)

## Errata

You'll notice two differences between the above schematic and the photo of the board above:

* I had originally intended to control the power supply to the attached GoPro using a mosfet (you'll see a jumper lead in the above photo between two of three leads nearby the USB-A connector.  It should've been clearer earlier, but I won't be able to detect the GoPro being plugged-in unless I'm supplying 5V; because the GoPro won't pull on the D+/D- lines until it is being supplied with voltage.
* I never populated the RGB LED in the bottom-left area of the board.  The footprint I've used in the above photo is essentially impossible to use on a milled board given the positioning of the pins on the SLV6A LED.  I've corrected the footprint in the attached project.
* You won't be able to tell in the above photo, but I had originally neglected to add a pulldown on GPIO15.  This caused the device to boot up into the wrong mode.  This bug took me a ton of time to find, and I only discovered what was wrong by connecting a logic analyzer to the ESP-12F's TX pin in autobaud mode to notice that the device was communicating its boot state after resetting.

## Future

* I'd probably not bother with the transitor-based reeset circuit.  The circuit in use here was copied from the NodeMCU schematic, and, although fairly simple, isn't really necessary given that PlatformIO's ESP8266 tooling supports using the DTR and RTS pins for directly controlling Reset and GPIO0 directly.
