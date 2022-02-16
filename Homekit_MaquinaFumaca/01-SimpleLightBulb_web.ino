#include "HomeSpan.h"         // HomeSpan sketches always begin by including the HomeSpan library
#include <WebServer.h>                    // include WebServer library
#include "DEV_Switch.h"

WebServer webServer(80);                  // create WebServer on port 80

nvs_handle switchNVS;                                                      // handle for NVS storage

int switch1 = 15;

struct {                                                                  // structure to store pin numbers and dimmable flag
  uint8_t pin=32;
  uint8_t on=0;
} switchData[1];


void setup() {                // Your HomeSpan code should be placed within the standard Arduino setup() function
 
  Serial.begin(115200);       // Start a serial connection so you can receive HomeSpan diagnostics and control the device using HomeSpan's Command-Line Interface (CLI)

  pinMode(switch1, OUTPUT); 
  digitalWrite(switch1, HIGH);

  homeSpan.setHostNameSuffix("");         // use null string for suffix (rather than the HomeSpan device ID)
  homeSpan.setPortNum(1201);              // change port number for HomeSpan so we can use port 80 for the Web Server
  homeSpan.enableOTA(false);                   // enable OTA updates
  homeSpan.setMaxConnections(5);          // reduce max connection to 5 (default is 8) since WebServer and a connecting client will need 2, and OTA needs 1
  homeSpan.setWifiCallback(setupWeb);  
  // homeSpan.setLogLevel(1);
  homeSpan.begin(Category::Switches,"Maquina Fumaca");   // initializes a HomeSpan device named "HomeSpan Lightbulb" with Category set to Lighting

  size_t len;  
  nvs_open("SWITCHS",NVS_READWRITE,&switchNVS);             // open LIGHTS NVS
  if(!nvs_get_blob(switchNVS,"SWITCHDATA",NULL,&len))       // if data found
    nvs_get_blob(switchNVS,"SWITCHDATA",&switchData,&len);   // retrieve data
  
  new SpanAccessory();                              // Begin by creating a new Accessory using SpanAccessory(), which takes no arguments
  
    new Service::AccessoryInformation();            // HAP requires every Accessory to implement an AccessoryInformation Service, which has 6 required Characteristics:
      new Characteristic::Name("Maquina Fumaca");      // Name of the Accessory, which shows up on the HomeKit "tiles", and should be unique across Accessories
      
                                                      
      new Characteristic::Manufacturer("Prophetic");   // Manufacturer of the Accessory (arbitrary text string, and can be the same for every Accessory)
      new Characteristic::SerialNumber("123-ABC");    // Serial Number of the Accessory (arbitrary text string, and can be the same for every Accessory)
      new Characteristic::Model("Maquina Fumaca");     // Model of the Accessory (arbitrary text string, and can be the same for every Accessory)
      new Characteristic::FirmwareRevision("0.1");    // Firmware of the Accessory (arbitrary text string, and can be the same for every Accessory)

      new Characteristic::Identify();                 // Create the required Identify

  
    new Service::HAPProtocolInformation();          // Create the HAP Protcol Information Service  
      new Characteristic::Version("1.0.0");           // Set the Version Characteristicto "1.1.0" as required by HAP


    new DEV_Switch(switch1);

  // server.begin();

} // end of setup()

void loop(){

  homeSpan.checkConnect();

  if (!homeSpan.connected) {
    homeSpan.setApSSID("Prophetic-0001");
    homeSpan.setApPassword("12345678");
    homeSpan.autoStartAPEnabled = true;
  }
  
  homeSpan.poll();         // run HomeSpan!
  switchData[0].on = !digitalRead(switch1);
  webServer.handleClient();    
} // end of loop()


void setupWeb(){
  Serial.print("Starting Light Server Hub...\n\n");
  webServer.begin();

  // Create web routines inline

  webServer.on("/", []() {
  
    String content = "<html><body><form action='/configure' method='POST'><p><b>Maquina Fumaca - Prophetic Church</b></p><br><br>";
    content += "<br><span style=\"color:black;\">Maquina : </span><span style=\"color:";
    if(switchData[0].on) {
    content += "green;\">Ligado";
    } else {
      content += "red;\">Desligado";
    }

    content += "<input type=\"hidden\" name=\"o\" value=\"";
    if(switchData[0].on) {
    content += "1";
    } else {
      content += "0";
    }
    content += "\" />";

    content += "<br><input type='submit' value='"; 
    if(switchData[0].on) {
      content += "Desligar";
    } else {
      content += "Ligar";
    }
    content += "'></form><br><br><br>";
    content += "<button onclick=\"document.location='/reboot'\">Reboot</button>";

    webServer.send(200, "text/html", content);
    
  });  

  webServer.on("/configure", []() {
   
    for(int i=0;i<webServer.args();i++){
      switch(webServer.argName(i).charAt(0)){
        case 'o':
          switchData[webServer.argName(i).substring(1).toInt()].on=webServer.arg(i).toInt();
          digitalWrite(switch1, webServer.arg(i).toInt());
          break;
      }
    }

    String content = "<html><body>Comando enviado com sucesso<br><br>";
    
    content += "<br><button onclick=\"document.location='/'\">Return</button> ";
    content += "<button onclick=\"document.location='/reboot'\">Reboot</button>";

    nvs_set_blob(switchNVS,"SWITCHDATA",&switchData,sizeof(switchData));        // update data
    nvs_commit(switchNVS);                                                   // commit to NVS

    webServer.send(200, "text/html", content);
  
  });

  webServer.on("/reboot", []() {
    
    String content = "<html><body>Rebooting!  Will return to configuration page in 10 seconds.<br><br>";
    content += "<meta http-equiv = \"refresh\" content = \"10; url = /\" />";
    webServer.send(200, "text/html", content);

    
    ESP.restart();
  });

} 