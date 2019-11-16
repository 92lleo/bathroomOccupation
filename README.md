# bathroomOccupation
The basic drive for this project was me getting my hands on an old traffic light and trying to use it somewhere. Bathroom occupation simply signals wheter the bathroom door is locked or not. A locked door results in a red traffic light. Parts: Pedastrian traffic light, esp8266, 2x 5v relais, 1x infrared proximity sensor. 
To make the project more energy efficient, the 230v light bulbs in the traffic light were changed to 12v led lights with the same socket size. The 12v power supply also powers the esp8266. When the bathroom is unoccupied for 2 hours, the lights will turn off completely. There is also a very basic web interface, which shows the current state of the door lock, the lights and the occupation/free time and uptime.
The project was created using the sloeber ide.

![](https://kuenzler.io/img/gh/2019-11-16%2013.44.27.jpg)
![](https://kuenzler.io/img/gh/2019-11-16%2013.44.36.jpg)
![](https://kuenzler.io/img/gh/2019-11-16%2013.44.13.jpg)
