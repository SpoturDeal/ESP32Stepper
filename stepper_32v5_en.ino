/*
 #ESP32 Web Server to Control Stepper motor
 new version 08 April 2018
 Board: DOIT ESP32 DEVKIT V1, 80Mhz, 4MB(32Mhz),921600 None op COM3
 Driver: A4988 Driver
 Stepper: type 17HS1362-P4130
*/
#include <WiFi.h>
#include <EEPROM.h>

int ZMax = 23;         // Top Endstop Pin
int ZMin = 22;         // Bottom Endstop Pin
const char* ssid     = "YOUR SSID";
const char* password = "YOUR WIFI PASSWORD";
int LED = 12;          // LED Feedback GPIO 12
int DirPin = 18;       // Direction GPIO
int StepPin = 19;      // Step GPIO
int EnablePin = 21;     // Stepper enable pin
int MicroStep1Pin = 34; // Stepper MS1
int Microstep2Pin = 35; // Stepper MS2
int Microstep3Pin = 32; // Stepper MS3
int motorSpeed = 1000;      // Set step delay for motor in microseconds (smaller is faster)
int currPos = 0;
int oneRotation = 3200; // 200 x 1.8 degrees per step = 360
bool Mechanical_Endstop = 1; // If using mechanical endstops set to 1. If using HALL sensor, set to 0
int maxSteps = 2300;   // maximum steps
int testVar = 0;       // to check if maxsteps is stored
bool debugPrint = false;
uint8_t EEPROMaddress = 130;

WiFiServer server(80);

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(32);
  pinMode(DirPin, OUTPUT);      // set Stepper direction pin mode  
  pinMode(StepPin, OUTPUT);     // set Stepper step mode
  pinMode(EnablePin, OUTPUT);   // set Stepper enable pin
  pinMode(MicroStep1Pin, OUTPUT);	//set Microstep 1 config
  pinMode(MicroStep2Pin, OUTPUT);	//set Microstep 2 config
  pinMode(MicroStep3Pin, OUTPUT);	//set Microstep 3 config
  pinMode(ZMax, INPUT);           // set top detection
  pinMode(ZMin, INPUT);           // set down detection
  pinMode(LED, OUTPUT);         // ready LED
  
  digitalWrite(MicroStep1Pin, LOW); // Initialized with microstepping off
  digitalWrite(MicroStep2Pin, LOW); // Initialized with microstepping off
  digitalWrite(MicroStep3Pin, LOW); // Initialized with microstepping off

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
    digitalWrite(EnablePin, LOW);
    digitalWrite(DirPin, LOW);
    for (int i=1; i <= doSteps; i++){ 
        currPos++;
        int dnPin = digitalRead(ZMin); 
        if (Mechanical_Endstop == 1 && dnPin == LOW || currPos >= maxSteps){
            currPos=maxSteps;
            break;
        }
        else if (Mechanical_Endstop == 0 && dnPin == HIGH || currPos >= maxSteps){
            currPos=maxSteps;
            break;
        }
        digitalWrite(StepPin, HIGH);
        delayMicroseconds(motorSpeed);
        digitalWrite(StepPin,LOW );
        delayMicroseconds(motorSpeed); 
    }
    digitalWrite(EnablePin, HIGH);
    if (debugPrint ==true){
       Serial.println("Down to position " + String(currPos));
    }
    stopRoll();
}
void rollUp(int doSteps) {
    digitalWrite(EnablePin, LOW);
    digitalWrite(DirPin, HIGH);
    for (int i=1; i <= doSteps; i++){
        currPos--; 
        int upPin = digitalRead(ZMax); 
        if (Mechanical_Endstop == 1 && upPin == LOW || currPos < 5){
          currPos=0;
          break;
        }
        else if (Mechanical_Endstop == 0 && upPin == HIGH || currPos < 5){
          currPos=0;
          break;
        }
        digitalWrite(StepPin, HIGH);
        delayMicroseconds(motorSpeed);
        digitalWrite(StepPin,LOW );
        delayMicroseconds(motorSpeed);
        
    }
    digitalWrite(EnablePin, HIGH);
    if (debugPrint ==true){
       Serial.println("up to position " + String(currPos));
    }
    stopRoll();
}
void stopRoll(){
    if (debugPrint ==true){
       Serial.println("Stop");
    }
    digitalWrite(EnablePin, LOW);
    digitalWrite(DirPin, HIGH);
    delay(1000);
    digitalWrite(EnablePin, HIGH);
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
