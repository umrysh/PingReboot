PingReboot
==============

PingReboot is an Arduino project that is connected inside your LAN with your router plugged into it's power outlet. It pings an outside IP every 5 minutes and if it cannot reach that IP for a period of 10 minutes it power cycles the power outlet. It then waits a set amount of time and tries to ping again. If Internet connectivity is restored it sends an email stating it had to reboot the router.

Also included is the file "remotepingreboot.htm". Upload this file to a website you control and PingReboot will check this file every 5 minutes to see if the value has changed (For example it may have changed from <0> to <1>). If it has changed it will force a reboot. You never know when you may need to force a remote router reboot, now you can by simply changing a file on your web server.

In other words, you will never again have to drive out to work in the middle of the night to reboot a frozen router :)


Hardware Used:
* [Arduino Uno USB Microcontroller Rev 3](http://www.robotshop.com/ca/en/arduino-uno-usb-microcontroller-rev-3.html)
* [Phidgets Dual Relay Board](http://www.robotshop.com/ca/en/phidgets-dual-relay-board.html)
* [Arduino Ethernet Shield](http://www.robotshop.com/ca/en/arduino-ethernet-shield.html)
* [OUTLET PWR NEMA 5-15R SNAPIN/IDC](http://www.digikey.com/product-search/en?x=20&y=18&KeyWords=Q228-ND)
* Old Computer LEDs
* Old Power Box

Software Installation Notes:
* Modify and upload "sendTheEmail.php" to your server.
* Modify the "serverName" variable in PingReboot.ino to match your server's address.
* Upload "remotepingreboot.htm" to your server.


Enjoy

Contributing
------------

Feel free to fork and send [pull requests](http://help.github.com/fork-a-repo/).  Contributions welcome.

Credit
------------

This script makes use of the [ICMP Ping Library](http://forum.arduino.cc/index.php/topic,8701.0.html) and the [Arduino Morse Library](https://github.com/markfickett/arduinomorse).

License
-------

This script is open source software released under the GNU GENERAL PUBLIC LICENSE V3.