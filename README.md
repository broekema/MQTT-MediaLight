MQTT MediaLight: A simple ESP8266 based RGB light controller. Subscribes
to three MQTT subjects, one for each color, and outputs an analog signal
on three pins to drive an RGB light strip.

This code runs on a EPS8266 board connected to a 12V RGB LED strip on pins D0 
through D2. To ensure sufficient current is available for the RGB strip, an 
ULN2003 Darlington array is used, as is shown in the picture below.

In the configuration used, a buck convertor is added to step down the 12V LED
power supply to 5V to power the ESP8266.

![MQTT MediaLight hardware](MQTT-MediaLight.svg)