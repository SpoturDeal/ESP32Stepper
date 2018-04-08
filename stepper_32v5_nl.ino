/*
 #ESP32 Web Server to Control Stepper motor
 new version 04 April 2018
 Board: DOIT ESP32 DEVKIT V1, 80Mhz, 4MB(32Mhz),921600 None op COM3
 Driver: A4988 Driver
 Stepper: type 17HS1362-P4130
*/
#include <WiFi.h>
#include <EEPROM.h>

const char* ssid     = "YOUR SSID";
const char* password = "YOUR WIFI PASSWORD";
int LED = 12;          // GPIO 12
int DirPin = 18;       // Richting GPIO
int StepPin = 19;      // Stap GPIO
int currPos = 0;
int oneRotation = 200; // 200 x 1.8 graden per stap = 360  
int maxSteps = 2300;   // maximum aantal stappen
int testVar = 0;
bool debugPrint = false;
uint8_t EEPROMaddress = 4;

#define chkUpPin 34

WiFiServer server(80);

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(32);
  pinMode(DirPin, OUTPUT);      // instellen richting pin mode
  pinMode(StepPin, OUTPUT);     // instellen stap mode
  pinMode(LED, OUTPUT);         // knipper LED als Network OK
  pinMode(chkUpPin,INPUT);

  // We starten met verbinden aan een WiFi netwerk
  Serial.println();
  Serial.print("Verbinden met ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi verbonden");
  Serial.println("IP adres: ");
  Serial.println(WiFi.localIP());
  Serial.println("Geef dit adres op in je Internet browser.");
 
  
  server.begin();
  // Check of het totaal aantal stappen is ingesteld
  EEPROM.get(EEPROMaddress,testVar);
  if (testVar > 0){
     EEPROM.get(EEPROMaddress,maxSteps);
  }
  // Breng het gordijn naar de begin stand open.
  currPos=maxSteps;
  rollDown(400);
  rollUp(maxSteps);
  // Laat LED knipperen om aan te geven dat initialisatie klaar is
  blink(3);
}

int value = 0;

void loop(){
 
  WiFiClient client = server.available();        // luisteren naar inkomende gebruiker
  if (client) {                             
    while (client.connected()) {            
      if (client.available()) {                 // als de gebruiker iets stuurt
        String respMsg = "Huidige positie = " + String(currPos);                    // HTTP Respons Bericht
        // Lees de eerste lijn van de aanvraag
        String req = client.readStringUntil('\r');
        if (debugPrint ==true){
           Serial.println(req);
        }
        client.flush();
        if (req.indexOf("/stepper/movedown") != -1) {
          int stepsToDo = getValue(req);
          if (stepsToDo < 1 || stepsToDo > maxSteps || stepsToDo == 0) {
             respMsg = "FOUT: stappen buiten bereik van 1 tot "+ String(maxSteps);
          } else {
             rollDown(stepsToDo);
             respMsg = "OK: Stappen gezet = " + String(stepsToDo) + " huidige positie =" + String(currPos);
          }
        } else if (req.indexOf("/stepper/moveup") != -1) {
          int stepsToDo = getValue(req);
          if (stepsToDo < 1 || stepsToDo > maxSteps || stepsToDo == 0) {
             respMsg = "FOUT: stappen buiten bereik van 1 tot "+ String(maxSteps);
          } else {
             rollUp(stepsToDo);
             respMsg = "OK: Stappen gezet = " + String(stepsToDo) + " huidige positie =" + String(currPos);
          }
        } else if (req.indexOf("/stepper/percent") != -1) {  
          int toPerc = getValue(req);
          if (toPerc > 100 || toPerc < 1) {
             respMsg = "FOUT: percentage buiten bereik van 1 tot 100";
          } else {
            int exSteps = ((maxSteps/100)*toPerc)-currPos;
            if (exSteps > 1){
              rollDown(exSteps);
              respMsg = "OK: Stappen omlaag gezet = " + String(exSteps) + " huidige positie =" + String(currPos);
            } else {
              exSteps=abs(exSteps);
              rollUp(exSteps);
              respMsg = "OK: Stappen omhoog gezet = " + String(exSteps) + " huidige positie =" + String(currPos);
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
          respMsg = "OK: Maximum stappen ingesteld op: " + String(maxSteps);
        }  
        //else {
          //respMsg = printUsage();
        //}
        client.flush();
        // Bereid antwoord voor
        String s = "<!DOCTYPE html><html><head><meta name='viewport' content='initial-scale=1.0'><meta charset='utf-8'><style>#map {height: 100%;}html, body {height: 100%;margin: 25px;padding: 10px;font-family: Sans-Serif;} p{font-family:'Courier New', Sans-Serif;}</style>";
        s += "<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css\">";
        s +="<script defer src=\"https://use.fontawesome.com/releases/v5.0.9/js/all.js\"></script></head>";
        s += "<body><h1>Rolgordijn besturing via WiFi</h1>";
        s += "<button type=\"button\" class=\"btn btn-primary btn-lg\" id=\"btn_down\"><i class=\"fas fa-arrow-alt-circle-down fa-2x\"></i></button>&nbsp;";
        s += "<button type=\"button\" class=\"btn btn-primary btn-lg\" id=\"btn_up\"><i class=\"fas fa-arrow-alt-circle-up fa-2x\"></i></button>&nbsp;";
        s += "<button type=\"button\" class=\"btn btn-dark btn-lg dropdown-toggle\" type=\"button\" id=\"ddMB\" data-toggle=\"dropdown\" aria-haspopup=\"true\" aria-expanded=\"false\"><i class=\"fas fa-percent fa-2x\"></i>&nbsp;</button><div class=\"dropdown-menu\" aria-labelledby=\"ddMB\"><a class=\"dropdown-item\" id=\"btn-10\" href=\"#\">10%</a><a class=\"dropdown-item\"id=\"btn-20\" href=\"#\">20%</a><a class=\"dropdown-item\" id=\"btn-30\" href=\"#\">30%</a>";
        s += "<a class=\"dropdown-item\" id=\"btn-40\" href=\"#\">40%</a><a class=\"dropdown-item\"id=\"btn-50\" href=\"#\">50%</a><a class=\"dropdown-item\" id=\"btn-60\" href=\"#\">60%</a>";
        s += "<a class=\"dropdown-item\" id=\"btn-70\" href=\"#\">70%</a><a class=\"dropdown-item\"id=\"btn-80\" href=\"#\">80%</a><a class=\"dropdown-item\" id=\"btn-90\" href=\"#\">90%</a>";
        s += "</div><br><br>";
        s += "<div id=\"w\" class=\"alert alert-success\" role=\"alert\" style=\"display:none;\">Een ogenblik a.u.b.</div>";
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
       Serial.println("Neer naar positie " + String(currPos));
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
       Serial.println("Omhoog naar positie " + String(currPos));
    }
    stopRoll();
    
}
void stopRoll(){
    if (debugPrint ==true){
       Serial.println("Stop");
    }
    digitalWrite(DirPin, HIGH);
    blink(1);
}
void blink(int blinks) {
  for (int i = 1; i <= blinks;i++){
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
        Serial.print("Ongeldige aanvraag: ");
        Serial.println(req);
     }
     return(0);
  }
  req = req.substring(val_start + 1, val_end);
  if (debugPrint ==true){
     Serial.print("Aangevraagde stappen/percentage: ");
     Serial.println(req);
  }
  return(req.toInt());
}
String printUsage() {
  // Bereid een gebruiksaanwijzing voor
  String s = "<p><u>Stapper gebruik</u><br><br>";
  s += "[Omhoog]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;http://{ip_address}/stepper/moveup?" + String(maxSteps) + "<br>";
  s += "[Omlaag]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;http://{ip_address}/stepper/movedown?" + String(maxSteps) + "<br>";
  s += "[Percentage]&nbsp;&nbsp;http://{ip_address}/stepper/percent?50<br>";
  s += "[Max stappen]&nbsp;http://{ip_address}/stepper/setup?2200<br><br><b>Maximum aantal stappen is " + String(maxSteps)+"<br>(percent) .. Percentage is 1 (Open) tot 100 (Dicht)</b></p>";
  return(s);
}
