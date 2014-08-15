3Sentry
=======

This project aims to write software to control a Sentry turret inspired in the Portal game series from Valve.

![Portal turret](http://i.imgur.com/sCmtvHc.png?1)

It's composed of two modules
* actuator: Software for the Arduino that controls servos, relays, etc
* detector: Software for the CPU that runs OpenCV-based pattern detection algorithms, in order to detect targets and command the actuator to shoot them.
