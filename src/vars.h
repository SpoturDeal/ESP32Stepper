// define variables

int ZMax = 23;         // Top Endstop Pin
int ZMin = 22;         // Bottom Endstop Pin
const char* ssid     = my_SSID;         // set in credentials.h     
const char* password = my_PASSWORD;     // set in credentials.h  
int LED = 12;          // LED Feedback GPIO 12
int DirPin = 18;       // Direction GPIO
int StepPin = 19;      // Step GPIO
int EnablePin = 21;     // Stepper enable pin
int MicroStep1Pin = 34; // Stepper MS1
int MicroStep2Pin = 35; // Stepper MS2
int MicroStep3Pin = 32; // Stepper MS3
int motorSpeed = 1000;      // Set step delay for motor in microseconds (smaller is faster)
int currPos = 0;
int oneRotation = 3200; // 200 x 1.8 degrees per step = 360
bool Mechanical_Endstop = 1; // If using mechanical endstops set to 1. If using HALL sensor, set to 0
int maxSteps = 2300;   // maximum steps
int testVar = 0;       // to check if maxsteps is stored
int value = 0;
bool debugPrint = false;

uint8_t EEPROMaddress = 130;
uint8_t EEmaxSteps = 4;
uint8_t EEcurrStep = 8;
uint8_t EEsetInit = 12;

String ipStr = "";
String respMsg = "";
String formattedTime = "";
bool noInit = true;