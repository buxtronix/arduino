# A(nother) NTP based clock for ESP8266 #

This is an LED clock driven by the amazing little ESP8266
device. There are a few people who have made NTP clients
for this device, but this one is thought out properly, and
much easier to setup and use:

* Uses the cheapest ESP8266 module (the ESP-1).
* Drives a 4-digit LED display via SPI.
* Single button to begin configuration.
* Browser based config entry.

The circuit is extremely basic, with the following parts:

* ESP-1 module.
* 7-segment LED display (SPI interface)
* 3.3v regulator (e.g LM1117)
* Bypass cap
* Button + pullup resistor.

The particular LED module I'm using is a Sure Electronics
display. It runs off 5v, however the driver chips are 3.3v
input compatible, obviating the need for a level converter.

## Operation ##

The clock has a simple interface, and does not require any software
changes to set it up for your network.

Setup:

* Power on.
* Within 5s press the button.
* Display shows 'AP'.
* Press the button to display the device IP address.
* Connect to the 'ESP-CLOCK' SSID.
* Point a browser to the device IP.
* Configure Wifi credentials and time/ntp attributes.
* After submitting, the clock connects to the given Wifi network and starts.

During normal operation:

* Press the button to display the device IP address.
* Browse to that address to view the status and change config.
* The lower right decimal point will be lit if NTP synchronisation
is overdue (e.g couldnt connect to server).

## Copyright ##

The design and code is Copyright 2015 Ben Buxton. (bbuxton@gmail.com).

Licenced under GPLv3.

