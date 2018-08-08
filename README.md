# ESP32Stepper for Domoticz with Web server
USE Stepper Motor with ESP32

Use a Steppermotor through Wifi and conrtol it with Domoticz or from your browser.
For use in Domoticz change the url to http://{ip_address}/api/stepper/..... this will result in a sinlge line reply

Version ending on <b>v5</b> has control through jQuery and layout with Bootstrap and Fontawesome.

## Updates
8th April 2018 -- Added EEprom store of maximum steps
4th April 2018 -- Graphical

## Screenshots
#### Display type: webserver
![Webserver](/shutterserver.png?raw=true "Included assets")

#### Display type: Arduino monitor
![Serail monitor](/shutterserial.png?raw=true "Included assets")


## Materials to use
Board: DOIT ESP32 DEVKIT V1, 80Mhz, 4MB(32Mhz),921600 None op COM3 <a href="https://www.banggood.com/ESP32-Development-Board-WiFiBluetooth-Ultra-Low-Power-Consumption-Dual-Cores-ESP-32-ESP-32S-Board-p-1109512.html?p=VQ141018240205201801">Available at Banggood</a>

 Driver: A4988 Driver <a href="https://www.banggood.com/3Pcs-3D-Printer-A4988-Reprap-Stepping-Stepper-Step-Motor-Driver-Module-p-967057.html?p=VQ141018240205201801">Available at Banggood</a>
 
 Stepper: type 17HS1362-P4130 <a href="https://www.banggood.com/NEMA-17-42-Hybrid-Stepper-Motor-5mm-Round-Shaft-Two-Phase-Four-Line-Stepper-Motor-p-1148356.html?p=VQ141018240205201801">Available at Banggood</a>
 
 HAL Sensor: ADAFruit Digital <a href="https://www.banggood.com/Hall-Effect-Magnetic-Sensor-Module-DC-5V-For-Arduino-p-76469.html?p=VQ141018240205201801">Available at Banggood</a> or at <a href="https://www.digikey.nl/products/en?keywords=OH090U">Digikey</a>
 
 Capacitor: 100uF, 25V
 
 PowerSupply(1):  5 Volt <a href="https://www.banggood.com/3Pcs-DC-DC-4_5-40V-Step-Down-LED-Voltmeter-USB-Voltage-Converter-Buck-Module-5V2A-p-1178249.html?p=VQ141018240205201801">Available at Banggood</a>
 
 PowerSupply(2): 12 Volt <a href="https://www.banggood.com/AC-100-240V-to-DC-12V-5A-60W-Power-Supply-Adapter-For-LED-Strip-Light-p-994870.html?p=VQ141018240205201801">Available at Banggood</a>
 
 ## Where to obtain the parts and materials
 Banggood is where I buy all my parts <a href="https://bit.ly/2IBUsds">Visit Banggood</a>

 ## Wired connections
 
|From|To|
|---|---|
|**ESP Module**||
|ESP32 VIN| -> +5V (1)|
|ESP32 GND| -> GROUND (1)|
|ESP32 D12| -> LED -> Resistor 1K -> Ground|
|ESP32 D18| -> A4988 Direction|
|ESP32 D19| -> A4988 Step|
|ESP32 D34| -> HAL Sensor D0|
|**HAL or REED Sensor**||
|HAL Sensor VCC| -> +5V (1)|
|HAL Sensor GND| -> GROUND (1)|
|**A4988 Stepper Module**|| 
|A4988 SLEEP| -> A4988 RESET|
|A4988 GND| -> GROUND (1)|
|A4988 VCC| -> 5V (1)|
|A4988 1B| -> Stepper RED|
|A4988 1A| -> Stepper BLUE|
|A4988 2A| -> Stepper GREEN|
|A4988 2B| -> Stepper BLACK|
|A4988 GND| -> GROUND 2|
|A4988 VMOT| -> +12V (2)|
|**Parts**||
|Capacitor+| -> +12V (2)|
|Capacitor-| -> GROUND (2)|

```
/*
 #ESP32 Web Server to Control Stepper motor
 new version 08 April 2018
 Board: DOIT ESP32 DEVKIT V1, 80Mhz, 4MB(32Mhz),921600 None op COM3
 Driver: A4988 Driver
 Stepper: type 17HS1362-P4130
*/

```
## Project Photo
#### Display type: All together
![Overview](/parts_built_together.png?raw=true "Included assets")

## MIT License

Copyright (c) 2018 SpoturDeal | Carl

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
