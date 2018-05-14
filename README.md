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
#include <WiFi.h>
#include <EEPROM.h>

const char* ssid     = "YOUR SSID";
const char* password = "YOUR WIFI PASSWORD";
int LED = 12;          // GPIO 12
int DirPin = 18;       // Direction GPIO
int StepPin = 19;      // Step GPIO
int currPos = 0;
int oneRotation = 200; // 200 x 1.8 degrees per step = 360
int maxSteps = 2300;   // maximum steps
int testVar = 0;       // to check if maxsteps is stored
bool debugPrint = false;
#define chkUpPin 34
uint8_t EEPROMaddress = 130;

WiFiServer server(80);

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(32);
  pinMode(DirPin, OUTPUT);      // set Stepper direction pin mode  
  pinMode(StepPin, OUTPUT);     // set Stepper step mode
  pinMode(22, INPUT);           // set top detection
  pinMode(23, INPUT);           // set down detection
  pinMode(LED, OUTPUT);         // ready LED
  pinMode(chkUpPin,INPUT);
 

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Place this IP address into a browser window");
  
 
  server.begin();
  // Check if the maximum number of steps has been stored in flash
  EEPROM.get(EEPROMaddress,testVar);
  if (testVar > 0){
     EEPROM.get(EEPROMaddress,maxSteps);
  }
  
  // move the shutter to start position
  currPos=maxSteps;
  rollDown(400);
  rollUp(maxSteps);
  // Blink onboard LED to signify its connected
  blink(3);

}

int value = 0;

void loop(){
 
  WiFiClient client = server.available();        // listen for incoming clients
  if (client) {                             
    while (client.connected()) {            
      if (client.available()) {                 // if there's client data
        String respMsg = "Current position = " + String(currPos); ;                    // HTTP Response Message
        // Read the first line of the request
        String req = client.readStringUntil('\r');
        if (debugPrint ==true){
           Serial.println(req);
        }
        client.flush();
        if (req.indexOf("/stepper/movedown") != -1) {
          int stepsToDo = getValue(req);
          if (stepsToDo < 1 || stepsToDo > maxSteps || stepsToDo == 0) {
             respMsg = "ERROR: steps out of range 1 to "+ String(maxSteps);
          } else {
             rollDown(stepsToDo);
             respMsg = "OK: Steps done = " + String(stepsToDo) + " current position =" + String(currPos);
          }
        } else if (req.indexOf("/stepper/moveup") != -1) {
          int stepsToDo = getValue(req);
          if (stepsToDo < 1 || stepsToDo > maxSteps || stepsToDo == 0) {
             respMsg = "ERROR: steps out of range 1 to "+ String(maxSteps);
          } else {
             rollUp(stepsToDo);
             respMsg = "OK: Steps done = " + String(stepsToDo) + " current position =" + String(currPos);
          }
        } else if (req.indexOf("/stepper/percent") != -1) {  
          int toPerc = getValue(req);
          if (toPerc > 100 || toPerc < 1) {
             respMsg = "ERROR: percentage out of range 1 to 100";
          } else {
            int exSteps = ((maxSteps/100)*toPerc)-currPos;
            if (exSteps > 1){
              rollDown(exSteps);
              respMsg = "OK: Steps done down = " + String(exSteps) + " current position =" + String(currPos);
            } else {
              exSteps=abs(exSteps);
              rollUp(exSteps);
              respMsg = "OK: Steps done up = " + String(exSteps) + " current position =" + String(currPos);
            }
          }
        } else if (req.indexOf("/stepper/setup") != -1) {  
          maxSteps = getValue(req);
          // Check if we don't store old info because EEPROM has limit 100.000 write/erase
          EEPROM.get(EEPROMaddress,testVar);
          if (maxSteps != testVar){
             EEPROM.put(EEPROMaddress,maxSteps);
             EEPROM.commit();
             Serial.print("setup/maxSteps EEPROM data (MaxSteps) at Address = "+String(EEPROMaddress)+" is  : ");
             testVar = 0; // To prove it read from EEPROM!
             EEPROM.get(EEPROMaddress,testVar);
             Serial.println(testVar);
          }
          respMsg = "OK: Maximum steps set at: " + String(maxSteps);
        }
        client.flush();
        // Prepare the response
        String s = "<!DOCTYPE html><html><head><meta name='viewport' content='initial-scale=1.0'><meta charset='utf-8'><style>#map {height: 100%;}html, body {height: 100%;margin: 25px;padding: 10px;font-family: Sans-Serif;} p{font-family:'Courier New', Sans-Serif;}</style>";
        s += "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css\">";
        s +="<script defer src=\"https://use.fontawesome.com/releases/v5.0.9/js/all.js\"></script></head>";        
        s += "<body><h1>Stepper control through WiFi</h1>";
        s += "<button type=\"button\" class=\"btn btn-primary btn-lg\" id=\"btn_down\"><i class=\"fas fa-arrow-alt-circle-down fa-2x\"></i></button>&nbsp;";
        s += "<button type=\"button\" class=\"btn btn-primary btn-lg\" id=\"btn_up\"><i class=\"fas fa-arrow-alt-circle-up fa-2x\"></i></button>&nbsp;";
        s += "<button type=\"button\" class=\"btn btn-dark btn-lg dropdown-toggle\" type=\"button\" id=\"ddMB\" data-toggle=\"dropdown\" aria-haspopup=\"true\" aria-expanded=\"false\"><i class=\"fas fa-percent fa-2x\"></i>&nbsp;</button><div class=\"dropdown-menu\" aria-labelledby=\"ddMB\"><a class=\"dropdown-item\" id=\"btn-10\" href=\"#\">10%</a><a class=\"dropdown-item\"id=\"btn-20\" href=\"#\">20%</a><a class=\"dropdown-item\" id=\"btn-30\" href=\"#\">30%</a>";
        s += "<a class=\"dropdown-item\" id=\"btn-40\" href=\"#\">40%</a><a class=\"dropdown-item\"id=\"btn-50\" href=\"#\">50%</a><a class=\"dropdown-item\" id=\"btn-60\" href=\"#\">60%</a>";
        s += "<a class=\"dropdown-item\" id=\"btn-70\" href=\"#\">70%</a><a class=\"dropdown-item\"id=\"btn-80\" href=\"#\">80%</a><a class=\"dropdown-item\" id=\"btn-90\" href=\"#\">90%</a>";
        s += "</div><br><br>";
        s += "<div id=\"w\" class=\"alert alert-success\" role=\"alert\" style=\"display:none;\">One moment please.</div>";
        s += "<div id=\"r\" class=\"alert alert-info\" role=\"alert\">" + respMsg + "</div>";
        s += printUsage();
        s +="<script src=\"https://code.jquery.com/jquery-3.2.1.min.js\"></script><script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.12.9/umd/popper.min.js\" ></script><script src=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/js/bootstrap.min.js\"></script>";
        s +="<script>$(document).ready(function($) {";
        s +="function send(dowhat){$('#r').hide(); $('#w').show(); $.get({url:'/api/stepper/' + dowhat,success:function(data){$('#w').hide(); $('#r').html(data).show(); }}); } ";
        s +="$('#btn_down').click(function(){send('movedown?" + String(maxSteps) + "');});";
        s +="$('#btn_up').click(function(){send('moveup?" + String(maxSteps) + "');});";
        s +="$('.dropdown-item').click(function(){ var per= $(this).attr('id').split('-');send('percent?'+per[1]);  });";
        s +="});";
        s += "</script></body></html>";
        // Stuur het antwoord naar de gebruiker
        if (req.indexOf("/api/") != -1){
          s = respMsg;
        }

        // Send the response to the client
        client.print(s);
        delay(1);
        break;
      }
    }
  }
}

void rollDown(int doSteps) {
    digitalWrite(DirPin, LOW);
    for (int i=1; i <= doSteps; i++){ 
        currPos++;
        if (currPos > maxSteps){
           break;
        }
        digitalWrite(StepPin, HIGH);
        delay(10);
        digitalWrite(StepPin,LOW );
        delay(10); 
    }
    if (debugPrint ==true){
       Serial.println("Down to position " + String(currPos));
    }
    stopRoll();
}
void rollUp(int doSteps) {
    digitalWrite(DirPin, HIGH);
    for (int i=1; i <= doSteps; i++){
        currPos--; 
        int upPin = digitalRead(chkUpPin); 
        if (upPin == HIGH || currPos < 5){
          currPos=0;
          break;
        }
        digitalWrite(StepPin, HIGH);
        delay(10);
        digitalWrite(StepPin,LOW );
        delay(10);
        
    }
    if (debugPrint ==true){
       Serial.println("up to position " + String(currPos));
    }
    stopRoll();
}
void stopRoll(){
    if (debugPrint ==true){
       Serial.println("Stop");
    }
    digitalWrite(DirPin, HIGH);
}
void blink(int blinks) {
  for (int i = 0; i <= blinks;i++){
    digitalWrite(LED, HIGH);
    delay(300); 
    digitalWrite(LED, LOW);
    delay(300);
  }
}
int getValue(String req) {
  int val_start = req.indexOf('?');
  int val_end   = req.length();
  if (val_start == -1 || val_end == -1) {
     if (debugPrint ==true){
        Serial.print("Invalid request: ");
        Serial.println(req);
     }
     return(0);
  }
  req = req.substring(val_start + 1, val_end);
  if (debugPrint ==true){
     Serial.print("Request steps: ");
     Serial.println(req);
  }
  return(req.toInt());
}
String printUsage() {
  // Prepare the usage response
  String s = "<p><u>Stepper usage</u><br><br>";
  s += "http://{ip_address}/stepper/moveup?" + String(maxSteps)+"<br>";
  s += "http://{ip_address}/stepper/movedown?" + String(maxSteps)+"<br>";
  s += "http://{ip_address}/stepper/percent?50<br>";
  s += "http://{ip_address}/stepper/setup?2200<br><br><b>Maximum number of steps is " + String(maxSteps)+"<br>Percent is 1 (open) to 100 (Closed)</b></p>";
  return(s);
}


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
