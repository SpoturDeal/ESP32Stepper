

//  here are the credentials you need in your sketch

//  ip_MQTT could be an IP address or a server name like broker.example.com
//  if you use a different port for your MQTT set the proper port
//  OTA makes it possible to update over the air without pressing a button


//                  WIFI           (leave empty if you want to set by browser)

#define my_SSID "***YOUR SSID***"           // ssid of your accesspoint
#define my_PASSWORD "***YOUR_PASSWORD***"  // password of access point

//                  ACCESS POINT

#define  ap_SSID  "Esp32AP_ROLL"  // not shown or unsecured when secret is too short
#define  ap_SECRET "hellolol"     // minimum length of secret is 8 characters

//                  MQTT

#define ip_MQTT "192.168.xxx.xxx"
#define port_MQTT 1883
#define my_MQTT_USER "***YOUR_MQTT_USERNAME" // user for the mqtt server
#define my_MQTT_PW "**YOUR_MQTT_PASSWORD"    // password of mqtt

//                   OTA

#define my_OTA_PW "***YOUR_OTA_PASSWORD"     // password for OTA updates


