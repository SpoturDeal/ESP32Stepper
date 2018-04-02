/*
 #ESP32 Web Server to Control Stepper motor
 new version 04 April 2018
 Board: DOIT ESP32 DEVKIT V1, 80Mhz, 4MB(32Mhz),921600 None op COM3
 Driver: A4988 Driver
 Stepper: type 17HS1362-P4130
*/
#include <WiFi.h>

const char* ssid     = "YOUR SSID";
const char* password = "YOUR WIFI PASSWORD";
int LED = 12;          // GPIO 12
int DirPin = 18;       // Direction GPIO
int StepPin = 19;      // Step GPIO
int currPos = 0;
int oneRotation = 200; // 200 x 1.8 degrees per step = 360
int maxSteps = 2300;   // maximum steps
bool debugPrint = false;
#define chkUpPin 34


WiFiServer server(80);

void setup()
{
  Serial.begin(115200);
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
  // Blink onboard LED to signify its connected
  blink(5);
  server.begin();
  // move the shutter to start position
  currPos=maxSteps;
  rollDown(400);
  rollUp(maxSteps);

}

int value = 0;

void loop(){
 
  WiFiClient client = server.available();        // listen for incoming clients
  if (client) {                             
    while (client.connected()) {            
      if (client.available()) {                 // if there's client data
        String respMsg = "";                    // HTTP Response Message
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
        } else {
          respMsg = printUsage();
        }
        client.flush();
        // Prepare the response
        String s = "<!DOCTYPE html><html><head><meta name='viewport' content='initial-scale=1.0'><meta charset='utf-8'><style>#map {height: 100%;}html, body {height: 100%;margin: 25;padding: 10;font-family: Sans-Serif;} p{font-family:'Courier New', Sans-Serif;}</style></head>";
        s += "<body><h1>Shutter control through WiFi</h1>";
        if (respMsg.length() > 0) {
           s += "<button onmousedown=location.href='/stepper/movedown?200'>Down [1 round]</button>&nbsp;Shutter&nbsp;";
           s += "<button onmousedown=location.href='/stepper/moveup?200'>Up [1 round]</button><br><br>";
           s += "<button onmousedown=location.href='/stepper/percent?60'>Percent close</button><br><br>";
           s += respMsg;
           s += "</body></html>";
        } else {
           s += "OK";
           s += "\n";
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
  s += "http://{ip_address}/stepper/moveup?200<br>";
  s += "http://{ip_address}/stepper/movedown?200<br>";
  s += "http://{ip_address}/stepper/percent?50<br><br><b>Maximum number of steps is " + String(maxSteps)+"<br>Percent is 1 (open) to 100 (Closed)</b></p>";
  return(s);
}
