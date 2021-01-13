/*
   ESP8266 Witty
   http://s.click.aliexpress.com/e/R7aieQ3fe

   + MCP23017
   + 8-channel relay control panel
   + Optocoupler Isolation Voltage Test Board 8 Channel AC 220V

   COMMAND: PUSH & SWITCH
   V 4.00
   
   Author: Zzuutt
   https://github.com/zzuutt/gladys-esp8266-Witty---Light-and-Socket

*/
#include "LittleFS.h"
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>      //https://github.com/dancol90/ESP8266Ping
#include <Ticker.h>           //https://github.com/esp8266/Arduino/tree/master/libraries/Ticker
#include <ArduinoJson.h>      //by Benoit Blanchon v5.13.5
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <WiFiManager.h>      //https://github.com/kentaylor/WiFiManager
#include <OneButton.h>        //https://github.com/mathertel/OneButton
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include "ESP8266TrueRandom.h" //https://github.com/marvinroger/ESP8266TrueRandom
#include <CircularBuffer.h>   //https://github.com/rlogiacco/CircularBuffer
#include "html/i2c_html.h"
#include "html/state_html.h"
#include "FTPServer.h"

Ticker blinker;
Adafruit_MCP23017 mcp;
FTPServer ftpSrv(LittleFS);

class deviceMCP {
  public:
    unsigned int pinOut, id, pinIn, group, type;
    char deviceName[32];
    deviceMCP(unsigned int arg1, unsigned int arg2, unsigned int arg3,const char arg4[32], unsigned int arg5, unsigned int arg6){
      pinOut = arg1;
      id = arg2;
      pinIn = arg3;
      strcpy(deviceName, arg4);
      group = arg5;
      type = arg6;  //SWITCH = 2 / PUSH = 1
    }
};

class deviceCMD {
  public:
    int deviceConcerned;
    bool state;
    String cmd;
    deviceCMD(int arg1, String arg2, bool arg3){
      deviceConcerned = arg1;
      cmd = arg2;
      state = arg3;
    }
};

String topPage;
String bottomPage;

String version_soft = "4.9.0F";
//define your default values here, if there are different values in config.json, they are overwritten.
char gladys_server[40];
char gladys_port[6] = "8080";
char gladys_token[50] = "YOUR_GLADYS_TOKEN";
//default custom static IP
char static_ip[16] = "0.0.0.0";
char static_gw[16] = "192.168.0.254";
char static_sn[16] = "255.255.255.0";
char static_dns[16] = "192.168.0.254";
IPAddress _ip,_gw,_sn,_dns;

deviceMCP allDeviceA[8] = {{0,0,8,"Device 1",0,1},{1,0,9,"Device 2",0,1},{2,0,10,"Device 3",0,1},{3,0,11,"Device 4",0,1},{4,0,12,"Device 5",0,1},{5,0,13,"Device 6",0,1},{6,0,14,"Device 7",0,1},{7,0,15,"Device 8",0,1}};
char * groupName[] = {(char *)"Aucun                          ",
                    (char *)"Salon                          ", 
                    (char *)"Salle à manger                 ",
                    (char *)"Cuisine                        ",
                    (char *)"Chambre 1                      ",
                    (char *)"Chambre 2                      ",
                    (char *)"Chambre 3                      ",
                    (char *)"Couloir                        ",
                    (char *)"Exterieur                      "};
bool sensorLight = true;
bool sensor = false;

CircularBuffer<deviceCMD*, 8> bufferCMD;

// Constants
const char* CONFIG_FILE_NETWORK = "/config_network.json";
const char* CONFIG_FILE = "/config_param.json";

//Variables
//PIN du boutton poussoir reset
const int BUTTON_PIN_RST = 4;
//Pin de la LED RGB couleur bleu
const int BLUE_PIN_LED = 13;
//Pin de la LED RGB couleur rouge
const int RED_PIN_LED = 15;
//Pin de la LED RGB couleur verte
const int GREEN_PIN_LED = 12;
//Pin du capteur de luminosite
const int LIGHT_PIN = A0;

//Pin du I2C
unsigned int I2C_SDA_PIN = 0;
unsigned int I2C_SCL_PIN = 5;
unsigned int I2C_ADDR = 7;
//Delay Push
unsigned int delayPush = 300;
// Interrupts from the MCP will be handled by this PIN
unsigned int interruptPin = 14;
volatile byte interruptCounter = 0;
int numberOfInterrupts = 0;
bool invertInputDataPortB = true;
byte lastPortB = 0b00000000;

bool sensorState = false;
int lightValue = 0;
int checkSensorPeriod = 1000;
unsigned long intervalCheck = 250;
unsigned long timeNow;
unsigned long timeNowPush;
unsigned long intervalCheckMCP = 50;

unsigned long timePing;
unsigned long intervalPing = 300000; //5mn
int timeRestart = 0; //1440;
int timeSendState = 0;

bool debugMode = false;
bool espStart = false;
bool error = false;

// Indicates whether ESP has WiFi credentials saved from previous session
bool initialConfig = false;
bool initialNetworkParam = false;
bool initialParam = false;

// Delay LongPress (in millisec)
int longPressDelay = 8000;  //8s
// UUIDs in binary form are 16 bytes long
byte uuidNumber[16]; 
String uuidStr;

// Function Prototypes
bool readConfigFile();
bool writeConfigFile();

OneButton button(BUTTON_PIN_RST, true);

// Set web server port number to 80
ESP8266WebServer server(80);

void red();
void blue();
void green();
void white();
void black();
void changeStateLed(int);
void blinkLED(int, int, int, float);
void clearConfig();
void resetConfig();
void ICACHE_RAM_ATTR handleInterrupt();
byte readPortB();
void readStatePortB();
void sendStatePortB_info();
bool sensorStateLCR();
int sendStateToGladys(bool, int);
bool readConfigNetworkFile();
bool readConfigFile();
bool writeNetworkConfigFile();
bool writeConfigFile();
void displayData();
void debugState();
bool determinateToken();
int determinateID();
int determinateDevice(int);
bool changeState(int, String, bool);
void pushCmd(int);
void switchCmd(int, bool);
void getHeatindex();
void scanPorts();
String check_if_exist_I2C();
void checkPort();
void sendConfig();
void sendHeaderAccess();
void saveConfig();
void sendLastValue();
void setupMCPInterrupts();
void activeInterrupt();
void deactiveInterrupt();


void red() {
  digitalWrite(RED_PIN_LED, HIGH);
  digitalWrite(GREEN_PIN_LED, LOW);
  digitalWrite(BLUE_PIN_LED, LOW);
}

void blue() {
  digitalWrite(RED_PIN_LED, LOW);
  digitalWrite(GREEN_PIN_LED, LOW);
  digitalWrite(BLUE_PIN_LED, HIGH);
}

void green() {
  digitalWrite(RED_PIN_LED, LOW);
  digitalWrite(GREEN_PIN_LED, HIGH);
  digitalWrite(BLUE_PIN_LED, LOW);
}

void white() {
  digitalWrite(RED_PIN_LED, HIGH);
  digitalWrite(GREEN_PIN_LED, HIGH);
  digitalWrite(BLUE_PIN_LED, HIGH);
}

void black() {
  digitalWrite(RED_PIN_LED, LOW);
  digitalWrite(GREEN_PIN_LED, LOW);
  digitalWrite(BLUE_PIN_LED, LOW);
}

void changeStateLed(int pin) {
  byte red = bitRead(pin, 0);
  byte green = bitRead(pin, 1);
  byte blue = bitRead(pin, 2);

  if (red) {
    digitalWrite(RED_PIN_LED, !(digitalRead(RED_PIN_LED)));  //Invert Current State of LED  
  }
  if (green) {
    digitalWrite(GREEN_PIN_LED, !(digitalRead(GREEN_PIN_LED)));  //Invert Current State of LED
  }  
  if (blue) {   
    digitalWrite(BLUE_PIN_LED, !(digitalRead(BLUE_PIN_LED)));  //Invert Current State of LED
  }
}

void blinkLED(int c_red = 0, int c_green = 0, int c_blue = 0, float every = 0.5) {
  blinker.detach();
  black();
  //Initialize Ticker every 0.5s
  int pin = c_red + (c_green * 2) + (c_blue * 4);
  //Serial.println("value color:"+String(pin));
  if (pin > 0){
    blinker.attach(every, changeStateLed, pin);
  }
}

void clearConfig() {
  // is configuration portal requested?
  Serial.println("Configuration portal requested");
  blinkLED(0, 0, 1, 0.2);
  //Local intialization. Once its business is done, there is no need to keep it around
  
  // Extra parameters to be configured
  // After connecting, parameter.getValue() will get you the configured value
  // Format: <ID> <Placeholder text> <default value> <length> <custom HTML> <label placement>
  WiFiManagerParameter custom_textIp("<p style=\"border-top: 1px black solid;padding: 5px;margin-top: 10px;\"><b>Network setting</b></p>");
  // IP 
  WiFiManagerParameter p_static_ip("static_ip", "IP static", static_ip, 16);
  WiFiManagerParameter p_static_gw("static_gw", "Gateway", static_gw, 16);
  WiFiManagerParameter p_static_sn("static_sn", "Subnet mask", static_sn, 16);
  WiFiManagerParameter p_static_dns("static_dns", "DNS", static_dns, 16);
  // Just a quick hint
  WiFiManagerParameter p_hintip("<small>*Hint: Leave the IP address empty or 0.0.0.0, if you want to use DHCP</small>");
  WiFiManagerParameter p_hint("<small>*Hint: if you want to reuse the currently active WiFi credentials, leave SSID and Password fields empty</small>");
 

  // Initialize WiFIManager
  WiFiManager wifiManager;
   
  //add all parameters here

  wifiManager.addParameter(&p_hint);
  
  wifiManager.addParameter(&custom_textIp);
  wifiManager.addParameter(&p_static_ip);
  wifiManager.addParameter(&p_hintip);
  wifiManager.addParameter(&p_static_gw);
  wifiManager.addParameter(&p_static_sn);
  wifiManager.addParameter(&p_static_dns);
   
  // Sets timeout in seconds until configuration portal gets turned off.
  // If not specified device will remain in configuration mode until
  // switched off via webserver or device is restarted.
  // wifiManager.setConfigPortalTimeout(600);

  // It starts an access point
  // and goes into a blocking loop awaiting configuration.
  // Once the user leaves the portal with the exit button
  // processing will continue
  if (!wifiManager.startConfigPortal()) {
    Serial.println("Not connected to WiFi but continuing anyway.");
  } else {
    // If you get here you have connected to the WiFi
    Serial.println("Connected...yeey :)");
    blinkLED(0,0,1,0.5);

    // Getting posted form values and overriding local variables parameters
    // Config file is written regardless the connection state
    strcpy(static_ip, p_static_ip.getValue());
    strcpy(static_gw, p_static_gw.getValue());
    strcpy(static_sn, p_static_sn.getValue());
    strcpy(static_dns, p_static_dns.getValue());

    blinkLED(0, 0, 0, 0.5);
    black();
    // Writing JSON config file to flash for next boot
    Serial.println("Save data....");
    if (writeNetworkConfigFile()) {
      Serial.println("data saved :) !");
      blue();
      delay(1000);
    } else {
      red();
    }
  }
  Serial.println("Restart.....");
  delay(1000);
  ESP.restart();
  delay(5000);

}

void resetConfig(){
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  blinkLED(1,0,0,0.2);
  delay(3000);
  ESP.reset();  
}

void setup() {
  // Put your setup code here, to run once
  Serial.begin(115200);
  Serial.println("\n Starting");

  // Initialize the LED digital pin as an output.
  pinMode(BLUE_PIN_LED, OUTPUT);
  pinMode(RED_PIN_LED, OUTPUT);
  pinMode(GREEN_PIN_LED, OUTPUT);
  // Initialize trigger pins
  pinMode(BUTTON_PIN_RST, INPUT_PULLUP);
  // Initailise Light sensor pin
  pinMode(LIGHT_PIN, INPUT);

  blinkLED(0, 1, 0, 0.5);

  button.attachDoubleClick(displayData);            // link the function to be called on a doubleclick event.
  button.attachClick(debugState);                  // link the function to be called on a singleclick event.
  button.setPressTicks(longPressDelay);
  button.attachLongPressStart(resetConfig);            // link the function to be called on a longpress event.

  // Mount the filesystem
  bool result = LittleFS.begin();
  Serial.println("LittleFS opened: " + result);
  initialNetworkParam = readConfigNetworkFile();
  if (!initialNetworkParam) {
    Serial.println("Failed to read network configuration file !!!");
    initialConfig = true;
  }

  WiFi.printDiag(Serial); //Remove this line if you do not want to see WiFi password printed
  if (WiFi.SSID() == "") {
    Serial.println("We haven't got any access point credentials, so get them now");
    initialConfig = true;
  } else {
    if (!initialConfig) {
      Serial.println("Init static IP :" + String(static_ip));
      if(strlen(static_ip) != 0 && strcmp(static_ip,"0.0.0.0") != 0) {
        //set static ip
        _ip.fromString(static_ip);
        _gw.fromString(static_gw);
        _sn.fromString(static_sn);
        _dns.fromString(static_dns);

        WiFi.config(_ip, _dns, _gw, _sn);
        Serial.println("Init static IP");
      }
      
      blinkLED(0, 1, 0, 0.5); // Turn LED off as we are not in configuration mode.
      WiFi.mode(WIFI_STA); // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
      unsigned long startedAt = millis();
      Serial.print("After waiting ");
      int connRes = WiFi.waitForConnectResult();
      float waited = (millis() - startedAt);
      Serial.print(waited / 1000);
      Serial.print(" secs in setup() connection result is ");
      Serial.println(connRes);
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    blinkLED(1, 0, 0, 0.5);
    Serial.println("Failed to connect, finishing setup anyway");
  } else {
    initialParam = readConfigFile();

    pinMode(interruptPin, INPUT_PULLUP);
    //Define I2C pins
    Wire.begin(I2C_SDA_PIN,I2C_SCL_PIN);
    // MCP23017 address
    mcp.begin(I2C_ADDR);
    setupMCPInterrupts();

    // enable interrupts before going to sleep/wait
    // And we setup a callback for the arduino INT handler.
    activeInterrupt();
                
    blinkLED(0, 0, 0, 0.5);
    if(!initialParam) {
      blinkLED(1,0,1,0.5);
    } else {
      white();
    }
    Serial.print("Local ip: ");
    Serial.println(WiFi.localIP());
    //all the sites which will be available
    server.on( "/", getHeatindex );
    server.on( "/scanI2C", scanPorts );
    server.on( "/checkport", checkPort );
    server.on( "/readPort", checkPort );
    server.on( "/sendconfig", sendConfig );
    server.on( "/saveconfig", saveConfig );
    server.on( "/readStatePort", sendLastValue );

    server.serveStatic("/js", LittleFS, "/js");
    server.serveStatic("/css", LittleFS, "/css");
    server.serveStatic("/fonts", LittleFS, "/fonts");
    server.serveStatic("/sys", LittleFS, "/index.html");
    server.serveStatic("/config", LittleFS, "/config.html");
    server.serveStatic("/command", LittleFS, "/gear.html");
  
    server.begin();
    espStart = 1;
    timePing = intervalPing + millis();

    ftpSrv.begin("admin","admin");    //username, password for ftp.
  }
}

void setupMCPInterrupts(){
    mcp.setupInterrupts(false,false,LOW);  //FALLING
    
    for (int i = 0; i < 8; i++){
      // MCP23017 port A -> Output
      mcp.pinMode(allDeviceA[i].pinOut, OUTPUT);
      mcp.digitalWrite(allDeviceA[i].pinOut, 1);
      // MCP23017 port B -> Input
      mcp.pinMode(allDeviceA[i].pinIn, INPUT);
      mcp.pullUp(allDeviceA[i].pinIn, HIGH);
    }

    // MCP23017 Interrupt test
    for (int i = 0; i < 8; i++){
      mcp.setupInterruptPin(allDeviceA[i].pinIn,CHANGE);  
    }
}

void activeInterrupt() {
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
}
void deactiveInterrupt() {
  detachInterrupt(interruptPin);
}

void ICACHE_RAM_ATTR handleInterrupt() {
  //detachInterrupt(interruptPin);
  interruptCounter++;
  if(debugMode){
    Serial.println("An interrupt has occurred. Counter: " + String(interruptCounter));
  }
}

void loop(void) {
  // is configuration portal requested?
  if (initialConfig) {
    clearConfig();
  }
  if (espStart && initialParam){
    //read state all port B
    lastPortB = readPortB(); //mcp.readGPIO(1);
    lastPortB = lastPortB ^ 0b11111111;
    readStatePortB();
    espStart = 0;
    timePing = intervalPing + millis();
  }
  if(debugMode){
    ftpSrv.handleFTP();
  }

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("WiFi deconnected");
    red();
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
      button.tick();
    }
    Serial.println("\nWiFi connected");
  }
  
  //process all the requests for the Webserver
  server.handleClient();    
  
  // Read state button
  button.tick();
  
  sensor = sensorStateLCR();
  if(!debugMode && !error){
    if(initialParam){
      if(sensor){
          white();
      } else {
        black();
      }
    }
  }

  if(!bufferCMD.isEmpty()){
    deviceCMD* devicecmd = bufferCMD.shift();
    if(debugMode){
      Serial.println("Exec command changeState device: " + String(devicecmd->deviceConcerned) + " cmd: " + devicecmd->cmd + " state: " + String(devicecmd->state));
    }
    changeState(devicecmd->deviceConcerned, devicecmd->cmd, devicecmd->state);
    delete devicecmd;
  }

  if(interruptCounter > 0){
    interruptCounter--;
    numberOfInterrupts++;
    //attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);
    if(debugMode){
      Serial.println("An interrupt has occurred. Total: " + String(numberOfInterrupts));
      Serial.println("-------- DEBUG --------");
      Serial.println("--Device state change--");
    }

    readStatePortB();
    
    if(debugMode){
      Serial.println("-------- ----- --------");
    }
  }

  timeNow = millis();
  if(timeNow >= timePing) {
    //bool ret = Ping.ping(WiFi.localIP(),1);
    if(Ping.ping(WiFi.gatewayIP())) {
      Serial.println("Ping Success!!");
      error = false;
      timeRestart = 0;
      timeSendState ++;
    } else {
      Serial.println("Ping Error :(");
      error = true;
      timeRestart++;
    }
    timePing = intervalPing + millis();
    red();
    
    if (timeRestart >= 5){
      ESP.restart();
    }
    if (timeSendState >= 30){  //30 mn
      timeSendState = 0; 
      sendStatePortB_info();
    }
  }

}

byte readPortB() {
  unsigned long timeCheck;
  byte valPortB;
  timeNow = millis();
  timeCheck = timeNow + intervalCheckMCP;
  while(timeNow < timeCheck){
    timeNow = millis();
    yield();  
  }
  //read state all port B
  valPortB = mcp.readGPIO(1);  
  return valPortB;
}

void readStatePortB() {
  byte statePortB = readPortB();
  //commpare last value and new value
  if((lastPortB != statePortB) || espStart){
    //search for the port that has changed
    for (int i=0; i<8; i++){
      byte stateBit = bitRead(statePortB, i);
      byte lastStateBit = bitRead(lastPortB, i);
      if(stateBit != lastStateBit){
        if(invertInputDataPortB) {
          stateBit = !stateBit;
        }
        //Send to Gladys
        if(initialParam) {
          if(strlen(gladys_server) != 0 && strcmp(gladys_server,"0.0.0.0") != 0) {
            yield();
            sendStateToGladys(stateBit, allDeviceA[i].id); 
          }
        }
      }
    }
    lastPortB = statePortB;
  }  
}

void sendStatePortB_info(){
  //read state all port B
  byte statePortB = readPortB();
    for (int i=0; i<8; i++){
      byte stateBit = bitRead(statePortB, i);
        if(invertInputDataPortB) {
          stateBit = !stateBit;
        }
        //Send to Gladys
        if(initialParam) {
          if(strlen(gladys_server) != 0 && strcmp(gladys_server,"0.0.0.0") != 0) {
            yield();
            sendStateToGladys(stateBit, allDeviceA[i].id); 
          }
        }
    }
}

bool sensorStateLCR(){
  if(sensorLight){
    unsigned long timeCheck;
    timeNow = millis();
    timeCheck = timeNow + intervalCheck;
    while(timeNow < timeCheck){
      timeNow = millis();
      yield();  
    }
    lightValue = analogRead(LIGHT_PIN);

    if(lightValue > 300){
      sensorState = true;
    } else {
      sensorState = false;
    }
  } else {
    sensorState = true;
  }
  return sensorState;
}

int sendStateToGladys(bool realState, int deviceTypeId){
  WiFiClient wificlient;
  HTTPClient http;
  String getData, link, payload;
  
  if(sensor && !debugMode){
    blinkLED(1, 1, 1, 0.2);
  } else {
    black();
  }
  
  getData = "?token=" + String(gladys_token) + "&devicetype=" + String(deviceTypeId) + "&value=" + String(realState);
  if(strlen(gladys_port) == 0) {
    strcpy(gladys_port, "80");
  }
  link = "http://" + String(gladys_server) + ":" + String(gladys_port) + "/devicestate/create" + getData;
  http.begin(wificlient, link);
  int httpCode = http.GET();
  if(httpCode >0) {
    payload = http.getString();
  }
  if(debugMode){
    Serial.println("Link: " + link);
    Serial.println("http code: " + httpCode);
    Serial.println("response: " + payload + "\n");
  }

  http.end();
  
  if(!debugMode){
    blinkLED(0,0,0,0.5);
  } else {
    blinkLED(0,1,0,0.1);
  }
  
  return httpCode;
}

bool readConfigNetworkFile() {
  // this opens the config file in read-mode
  File f = LittleFS.open(CONFIG_FILE_NETWORK, "r");

  if (!f) {
    Serial.println("Configuration file not found");
    return false;
  } else {
    // we could open the file
    size_t size = f.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // Read and store file contents in buf
    f.readBytes(buf.get(), size);
    // Closing file
    f.close();
    // Using dynamic JSON buffer which is not the recommended memory model, but anyway
    // See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model

    DynamicJsonBuffer jsonBuffer;
    // Parse JSON string
    JsonObject& json = jsonBuffer.parseObject(buf.get());
    // Test if parsing succeeds.
    if (!json.success()) {
      Serial.println("JSON parseObject() failed");
      return false;
    }
    json.printTo(Serial);

    // Parse all config file parameters, override
    // local config variables with parsed values
    if (json.containsKey("static_ip")) {
      strcpy(static_ip, json["static_ip"]);
    }

    if (json.containsKey("static_gw")) {
      strcpy(static_gw, json["static_gw"]);
    }

    if (json.containsKey("static_sn")) {
      strcpy(static_sn, json["static_sn"]);
    }
    
    if (json.containsKey("static_dns")) {
      strcpy(static_dns, json["static_dns"]);
    }
  }
  Serial.println("\nConfig file was successfully parsed");
  return true;
}

bool readConfigFile() {
  Serial.println("Read configuration parameter file");
  // this opens the config file in read-mode
  File f = LittleFS.open(CONFIG_FILE, "r");

  if (!f) {
    Serial.println("Configuration file not found");
    return false;
  } else {
    // we could open the file
    size_t size = f.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // Read and store file contents in buf
    f.readBytes(buf.get(), size);
    // Closing file
    f.close();
    // Using dynamic JSON buffer which is not the recommended memory model, but anyway
    // See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model

    DynamicJsonBuffer jsonBuffer;
    // Parse JSON string
    JsonObject& json = jsonBuffer.parseObject(buf.get());
    // Test if parsing succeeds.
    if (!json.success()) {
      Serial.println("JSON parseObject() failed");
      return false;
    }
    json.printTo(Serial);

    // Parse all config file parameters, override
    // local config variables with parsed values     
    if (json.containsKey("gladys_server")) {
      strcpy(gladys_server, json["gladys_server"]);
    }

    if (json.containsKey("gladys_port")) {
      strcpy(gladys_port, json["gladys_port"]);
    }

    if (json.containsKey("gladys_token")) {
      strcpy(gladys_token, json["gladys_token"]);
    }

    if (json.containsKey("invertInputDataPortB")) {
      invertInputDataPortB = json["invertInputDataPortB"];
    }

    if (json.containsKey("sensorLight")) {
      sensorLight = json["sensorLight"];
    }
    
    if (json.containsKey("interruptPin")) {
      interruptPin = json["interruptPin"];
    }
    
    if (json.containsKey("I2C_SDA_PIN")) {
      I2C_SDA_PIN = json["I2C_SDA_PIN"];
    }
    if (json.containsKey("I2C_SCL_PIN")) {
      I2C_SCL_PIN = json["I2C_SCL_PIN"];
    }
    if (json.containsKey("I2C_ADDR")) {
      I2C_ADDR = json["I2C_ADDR"];
    }
    
    if (json.containsKey("Group_name1")) {
      strcpy(groupName[1], json["Group_name1"]);
    }
    if (json.containsKey("Group_name2")) {
      strcpy(groupName[2], json["Group_name2"]);
    }
    if (json.containsKey("Group_name3")) {
      strcpy(groupName[3], json["Group_name3"]);
    }
    if (json.containsKey("Group_name4")) {
      strcpy(groupName[4], json["Group_name4"]);
    }
    if (json.containsKey("Group_name5")) {
      strcpy(groupName[5], json["Group_name5"]);
    }
    if (json.containsKey("Group_name6")) {
      strcpy(groupName[6], json["Group_name6"]);
    }
    if (json.containsKey("Group_name7")) {
      strcpy(groupName[7], json["Group_name7"]);
    }
    if (json.containsKey("Group_name8")) {
      strcpy(groupName[8], json["Group_name8"]);
    }
    
    if (json.containsKey("ID_device1")) {
      allDeviceA[0].id = json["ID_device1"];
    }
    if (json.containsKey("ID_device2")) {
      allDeviceA[1].id = json["ID_device2"];
    }
    if (json.containsKey("ID_device3")) {
      allDeviceA[2].id = json["ID_device3"];
    }
    if (json.containsKey("ID_device4")) {
      allDeviceA[3].id = json["ID_device4"];
    }
    if (json.containsKey("ID_device5")) {
      allDeviceA[4].id = json["ID_device5"];
    }
    if (json.containsKey("ID_device6")) {
      allDeviceA[5].id = json["ID_device6"];
    }
    if (json.containsKey("ID_device7")) {
      allDeviceA[6].id = json["ID_device7"];
    }
    if (json.containsKey("ID_device8")) {
      allDeviceA[7].id = json["ID_device8"];
    }

    if (json.containsKey("NAME_device1")) {
      strcpy(allDeviceA[0].deviceName, json["NAME_device1"]);
    }
    if (json.containsKey("NAME_device2")) {
      strcpy(allDeviceA[1].deviceName, json["NAME_device2"]);
    }
    if (json.containsKey("NAME_device3")) {
      strcpy(allDeviceA[2].deviceName, json["NAME_device3"]);
    }
    if (json.containsKey("NAME_device4")) {
      strcpy(allDeviceA[3].deviceName, json["NAME_device4"]);
    }
    if (json.containsKey("NAME_device5")) {
      strcpy(allDeviceA[4].deviceName, json["NAME_device5"]);
    }
    if (json.containsKey("NAME_device6")) {
      strcpy(allDeviceA[5].deviceName, json["NAME_device6"]);
    }
    if (json.containsKey("NAME_device7")) {
      strcpy(allDeviceA[6].deviceName, json["NAME_device7"]);
    }
    if (json.containsKey("NAME_device8")) {
      strcpy(allDeviceA[7].deviceName, json["NAME_device8"]);
    }

    if (json.containsKey("Group_device1")) {
      allDeviceA[0].group = json["Group_device1"];
    }
    if (json.containsKey("Group_device2")) {
      allDeviceA[1].group = json["Group_device2"];
    }
    if (json.containsKey("Group_device3")) {
      allDeviceA[2].group = json["Group_device3"];
    }
    if (json.containsKey("Group_device4")) {
      allDeviceA[3].group = json["Group_device4"];
    }
    if (json.containsKey("Group_device5")) {
      allDeviceA[4].group = json["Group_device5"];
    }
    if (json.containsKey("Group_device6")) {
      allDeviceA[5].group = json["Group_device6"];
    }
    if (json.containsKey("Group_device7")) {
      allDeviceA[6].group = json["Group_device7"];
    }
    if (json.containsKey("Group_device8")) {
      allDeviceA[7].group = json["Group_device8"];
    }

    if (json.containsKey("TYPE_device1")) {
      allDeviceA[0].type = json["TYPE_device1"];
    }
    if (json.containsKey("TYPE_device2")) {
      allDeviceA[1].type = json["TYPE_device2"];
    }
    if (json.containsKey("TYPE_device3")) {
      allDeviceA[2].type = json["TYPE_device3"];
    }
    if (json.containsKey("TYPE_device4")) {
      allDeviceA[3].type = json["TYPE_device4"];
    }
    if (json.containsKey("TYPE_device5")) {
      allDeviceA[4].type = json["TYPE_device5"];
    }
    if (json.containsKey("TYPE_device6")) {
      allDeviceA[5].type = json["TYPE_device6"];
    }
    if (json.containsKey("TYPE_device7")) {
      allDeviceA[6].type = json["TYPE_device7"];
    }
    if (json.containsKey("TYPE_device8")) {
      allDeviceA[7].type = json["TYPE_device8"];
    }
    
    if (json.containsKey("delayPush")) {
      delayPush = json["delayPush"];
    }
  }
  Serial.println("\nConfig file was successfully parsed");
  return true;
}

bool writeNetworkConfigFile() {
  Serial.println("Saving network config file");

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  // JSONify local configuration parameters
  json["static_ip"] = static_ip;
  json["static_gw"] = static_gw;
  json["static_sn"] = static_sn;
  json["static_dns"] = static_dns;

  // Open file for writing
  File f = LittleFS.open(CONFIG_FILE_NETWORK, "w");
  if (!f) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.prettyPrintTo(Serial);
  // Write data to file and close it
  json.printTo(f);
  f.close();

  Serial.println("\nNetwork config file was successfully saved");
  return true;
}

bool writeConfigFile() {
  Serial.println("Saving config file");

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  // JSONify local configuration parameters
   
  json["gladys_server"] = gladys_server;
  json["gladys_port"] = gladys_port;
  json["gladys_token"] = gladys_token;
  json["invertInputDataPortB"] = invertInputDataPortB;
  json["sensorLight"] = sensorLight;
  json["interruptPin"] = interruptPin;
  json["I2C_SDA_PIN"] = I2C_SDA_PIN;
  json["I2C_SCL_PIN"] = I2C_SCL_PIN;
  json["I2C_ADDR"] = I2C_ADDR;

  json["Group_name1"] = groupName[1];
  json["Group_name2"] = groupName[2];
  json["Group_name3"] = groupName[3];
  json["Group_name4"] = groupName[4];
  json["Group_name5"] = groupName[5];
  json["Group_name6"] = groupName[6];
  json["Group_name7"] = groupName[7];
  json["Group_name8"] = groupName[8];
  
  json["ID_device1"] = allDeviceA[0].id;
  json["ID_device2"] = allDeviceA[1].id;
  json["ID_device3"] = allDeviceA[2].id;
  json["ID_device4"] = allDeviceA[3].id;
  json["ID_device5"] = allDeviceA[4].id;
  json["ID_device6"] = allDeviceA[5].id;
  json["ID_device7"] = allDeviceA[6].id;
  json["ID_device8"] = allDeviceA[7].id;

  json["NAME_device1"] = allDeviceA[0].deviceName;
  json["NAME_device2"] = allDeviceA[1].deviceName;
  json["NAME_device3"] = allDeviceA[2].deviceName;
  json["NAME_device4"] = allDeviceA[3].deviceName;
  json["NAME_device5"] = allDeviceA[4].deviceName;
  json["NAME_device6"] = allDeviceA[5].deviceName;
  json["NAME_device7"] = allDeviceA[6].deviceName;
  json["NAME_device8"] = allDeviceA[7].deviceName;

  json["TYPE_device1"] = allDeviceA[0].type;
  json["TYPE_device2"] = allDeviceA[1].type;
  json["TYPE_device3"] = allDeviceA[2].type;
  json["TYPE_device4"] = allDeviceA[3].type;
  json["TYPE_device5"] = allDeviceA[4].type;
  json["TYPE_device6"] = allDeviceA[5].type;
  json["TYPE_device7"] = allDeviceA[6].type;
  json["TYPE_device8"] = allDeviceA[7].type;
  
  json["Group_device1"] = allDeviceA[0].group;
  json["Group_device2"] = allDeviceA[1].group;
  json["Group_device3"] = allDeviceA[2].group;
  json["Group_device4"] = allDeviceA[3].group;
  json["Group_device5"] = allDeviceA[4].group;
  json["Group_device6"] = allDeviceA[5].group;
  json["Group_device7"] = allDeviceA[6].group;
  json["Group_device8"] = allDeviceA[7].group;

  json["delayPush"] = delayPush;

  // Open file for writing
  File f = LittleFS.open(CONFIG_FILE, "w");
  if (!f) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  json.prettyPrintTo(Serial);
  // Write data to file and close it
  json.printTo(f);
  f.close();

  Serial.println("\nConfig file was successfully saved");
  return true;
}

void displayData(){
  if(debugMode){
    Serial.println("\n............DATAS..........\n");
    Serial.print("Local ip: ");
    Serial.println(WiFi.localIP());
    Serial.print("Subnet mask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("\nGladys server: \n   IP: ");
    Serial.println(gladys_server);
    Serial.print("   Port server: ");
    Serial.println(gladys_port);
    Serial.print("   Token: ");
    Serial.println(gladys_token);
    Serial.print("\nSensor Light is used: ");
    if(sensorLight){
      Serial.print("true");
    } else {
      Serial.print("false");
    }
    Serial.println("  Value: " + String(lightValue));
    Serial.print("\nI2C: \n   SDA Pin: ");
    Serial.println(I2C_SDA_PIN);
    Serial.print("   SCL Pin: ");
    Serial.println(I2C_SCL_PIN);
    Serial.print("   Addr: ");
    Serial.println(I2C_ADDR);
    Serial.print("   Interrupt Pin: ");
    Serial.print(interruptPin);
    Serial.println("    Number of interrupts: " + String(numberOfInterrupts));
    Serial.println("\nCommand Push\n  pressure duration: " + String(delayPush) + " milliseconds");
    Serial.print("\nID Device: \n");
    for (int i = 0; i < 8; i++){
      Serial.println("   " + String(i+1) + ":  -Pin In: " + String(allDeviceA[i].pinIn + 1) + "  -ID: " + allDeviceA[i].id + "  -Pin Out: " + String(allDeviceA[i].pinOut + 1) + "  -Name: " + String(allDeviceA[i].deviceName) + "  -Type: " + allDeviceA[i].type);
    }
    Serial.print("\nInvert input data PORTB: ");
    if(invertInputDataPortB){
      Serial.println("true");
    } else {
      Serial.println("false");
    }
    Serial.print("State of the Port B: ");
    byte value_lastPortB = lastPortB;
    if(invertInputDataPortB) {
          value_lastPortB = ~value_lastPortB;
    }
    Serial.println(value_lastPortB,BIN);
    Serial.println("\n............DATAS..........\n");
  }
}

void debugState(){
  debugMode = !(debugMode);
  if(!debugMode){
    blinkLED(0,0,0,0.5);
    Serial.println("--- DEBUG ***");
  } else {
    blinkLED(0,1,0,0.1);
    Serial.println("*** DEBUG ---");
  }
}

bool determinateToken() {
  int readInf = 0;
  if (server.hasArg("token")) { //just do the checks if the parameter is available
      if ( String(gladys_token) == server.arg("token")){
          readInf = 1;
      } else if(debugMode){
        Serial.println("ERROR Token");
        Serial.println("Token received: " + server.arg("token"));
        Serial.println("Token saved   : " + String(gladys_token));
      }
  }
  return readInf;
}

int determinateID() {
  int readInf = 0;
  if (server.hasArg("deviceid")) { //just do the checks if the parameter is available
      readInf = server.arg("deviceid").toInt();
  }
  if(debugMode){
    Serial.println("Device concerned ID: " + String(readInf));
  }
  return readInf;
}

int determinateDevice(int deviceID){
  int i, findDevice = 99;
  for(i=0; i < 8; i++){
    if(deviceID == (int)(allDeviceA[i].id)){
      findDevice = i;
      break;
    }
  }
  if(debugMode){
    Serial.println("Device ID: " + String(deviceID));
    Serial.println("     port: " + String(findDevice));
  }
  
  return findDevice;
}

bool changeState(int deviceConcerned, String cmdExec, bool currentState){
  bool realState = 0;
  //read real state
  //realState = mcp.digitalRead(allDeviceA[deviceConcerned].pinIn);  //not work why ????
  //read state all port B
  byte statePortB = readPortB(); //mcp.readGPIO(1);
  realState = bitRead(statePortB, (allDeviceA[deviceConcerned].pinIn - 8));
  if(invertInputDataPortB) {
    realState = !realState;
  }
  if(debugMode){
    Serial.println("Order " + String(cmdExec) + " change state pinOut: " + String(allDeviceA[deviceConcerned].pinOut) + " pinIn: " + String(allDeviceA[deviceConcerned].pinIn) + " realstate: " + String(realState) + " currentState: " + String(currentState));
  }
  if (realState == currentState){
    unsigned long timeCheck;
    timeNow = millis();
    timeCheck = timeNow + intervalCheckMCP;
    while(timeNow < timeCheck){
      timeNow = millis();
      yield();  
    }
    if (cmdExec == "PUSH") {
        //exec push command
        pushCmd(deviceConcerned);
        return true;
    } else if(cmdExec == "SWITCH") {
        //exec switch command
        switchCmd(deviceConcerned, currentState);
        return true;
    }
  } else if(debugMode){
    Serial.println("The real state does not match the current state");
  }
  return false;
}

void pushCmd(int deviceConcerned){
  unsigned long timeCheckPush;
  if(debugMode){
    Serial.println("--- PUSH ---");
    Serial.println("Order PUSH change state pin: " + String(allDeviceA[deviceConcerned].pinOut) + " state: 1");
  }
  mcp.digitalWrite(allDeviceA[deviceConcerned].pinOut, 0);
  timeNowPush = millis();
  timeCheckPush = timeNowPush + delayPush;
  while(timeNowPush < timeCheckPush){
    timeNowPush = millis();
    yield();  
  }
  if(debugMode){
    Serial.println("Order PUSH change state pin: " + String(allDeviceA[deviceConcerned].pinOut) + " state: 0");
  }
  mcp.digitalWrite(allDeviceA[deviceConcerned].pinOut, 1);
}

void switchCmd(int deviceConcerned, bool state){
  if(debugMode){
    Serial.println("--- SWITCH ---");
    Serial.println("Order SWITCH change state pin: " + String(allDeviceA[deviceConcerned].pinOut) + " state: " + String(state));
  }
  mcp.digitalWrite(allDeviceA[deviceConcerned].pinOut, state);
}

void getHeatindex() {
  String cmd = "";
  bool state = 0;
  if(debugMode){
    Serial.println("-------- DEBUG --------");
  }
  bool tokenOk = determinateToken();

  if (tokenOk){
    int readArgId = determinateID();
    int deviceConcerned = determinateDevice(readArgId);
    if(deviceConcerned < 8){
      if (server.hasArg("cmd")) { //just do the checks if the parameter is available
        cmd = server.arg("cmd");
      }
      if (server.hasArg("state")) { //just do the checks if the parameter is available
        String stateString = server.arg("state");
        state = stateString.toInt();
      }    
    }
    if (cmd != ""){
      if(debugMode){
        Serial.println("Order received:");
        Serial.println("deviceID: " + String(deviceConcerned) + " cmd: " + cmd + " state: " + String(state));
      }
      deviceCMD* devicecmd = new deviceCMD(deviceConcerned, cmd, !state);
      bufferCMD.unshift(devicecmd);
      //changeState(deviceConcerned, cmd, !state); 
    }
    server.send ( 200, "application/json", "{\"deviceID\":\"" + String(readArgId) + "\",\"pin\":\"" + String(deviceConcerned) + "\",\"cmd\":\"" + cmd + "\",\"state\":\"" + String(state) + "\"}" );
  }
  if(debugMode){
    Serial.println("-------- DEBUG --------");
  }
  server.send(404, "text/plain", "404: Not found");
}

void scanPorts() {
  if(debugMode){
    deactiveInterrupt();
    String sendWebPage, checkI2C;
    uint8_t portArray[] = {0, 2, 5, 14, 16};
    String portMap[] = {"GPIO0", "GPIO2", "GPIO5", "GPIO14", "GPIO16"};
    sendWebPage = "<thead><tr><th><span class='colorblue'>SDA</span>\\<span class='colororange'>SCL</span></th>";
    for (uint8_t j = 0; j < sizeof(portArray); j++) {
      sendWebPage += "<th class='colororange'>" + portMap[j] + "</th>";
    }
    sendWebPage += "</tr></thead><tbody>";
    Serial.println("\n------- I2C Scanner -------");
    for (uint8_t i = 0; i < sizeof(portArray); i++) {
      sendWebPage += "<tr><th class='colorblue'>" + portMap[i] + "</th>";
      for (uint8_t j = 0; j < sizeof(portArray); j++) {
        if (i != j){
          Serial.print("Scanning (SDA : SCL) - " + portMap[i] + " : " + portMap[j] + " - ");       
          Wire.begin(portArray[i], portArray[j]);
          checkI2C = check_if_exist_I2C();
          sendWebPage += checkI2C;
        } else {
          sendWebPage += "<td> - </td>";  
        }
      }
      sendWebPage += "</tr>";
    }
    Serial.println("\n------- I2C Scanner -------");
    sendWebPage += "</tbody>";
    String pageFinal = FPSTR(I2C_page);
    pageFinal.replace("%%DATA%%", sendWebPage);
    server.send(200, "text/html", pageFinal);
    Wire.begin(I2C_SDA_PIN,I2C_SCL_PIN);
    activeInterrupt();
  } else {
    server.send(404, "text/plain", "404: Not found");
  }
}

String check_if_exist_I2C() {
  byte error, address;
  int nDevices;
  String sendWebPage = "";
  nDevices = 0;
  for (address = 1; address < 127; address++ )  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0){
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println("  !");
      sendWebPage += "<td class='ok'>0x" + String(address, HEX) + "</td>";
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknow error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      sendWebPage += "<td class='error'>ERROR 0x" + String(address, HEX) + "</td>";
    }
  } //for loop
  if (nDevices == 0) {
    Serial.println(" - No I2C devices found");
    sendWebPage += "<td>-</td>";
  } else {
    Serial.println("**********************************");
  }
  //delay(500);           // wait 1 seconds for next scan, did not find it necessary
  return sendWebPage;
}

void checkPort(){
  bool modeAjax = false;
  if (server.hasArg("mode")) { //just do the checks if the parameter is available
    modeAjax = server.arg("mode");
  }
  if(debugMode){
    String sendWebPage = "<table><tbody>";
    //read state all port B
    byte statePortB = readPortB(); //mcp.readGPIO(1);
    //search for the port that has changed
    for (int i=0; i<8; i++){
      byte stateBit = bitRead(statePortB, i);
      if(invertInputDataPortB) {
        stateBit = !stateBit;
      }
      sendWebPage += "<tr><td> Port: " + String(i) + " ID: " + String(allDeviceA[i].id) + "</td><td>" + stateBit + "</td></tr>";
    }
    sendWebPage += "</tbody></table>";
    if(modeAjax) {
      server.send(200, "text/html", sendWebPage);   
    } else { 
      String pageFinal = FPSTR(STATE_page);
      pageFinal.replace("%%DATA%%", sendWebPage);
      server.send(200, "text/html", pageFinal);
    }   
  } else {
    server.send(404, "text/plain", "404: Not found");
  }
}

void sendConfig(){
  bool stateCommand = false;
  if(server.hasArg("mode")) {
    if(server.arg("mode") == "direct") {
      stateCommand = true;       
    }
  }
  if(debugMode || stateCommand){
    Serial.println("Generate config file");
    // Generate a new UUID
    ESP8266TrueRandom.uuid(uuidNumber);
    uuidStr = ESP8266TrueRandom.uuidToString(uuidNumber);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();

    json["version"] = version_soft;

    if(!stateCommand && debugMode) {
      json["i2cSda"] = I2C_SDA_PIN;
      json["i2cScl"] = I2C_SCL_PIN;
      json["i2cAddr"] = I2C_ADDR;
      json["i2cIntB"] = interruptPin;
  
      json["delayPush"] = delayPush;
      json["sensorLight"] = sensorLight;
      json["invertInputDataPortB"] = invertInputDataPortB;
      
      json["gladys_server"] = gladys_server;
      json["gladys_port"] = gladys_port;
    }
    
    json["gladys_token"] = gladys_token; 
  
    json["groupName1"] = groupName[1];
    json["groupName2"] = groupName[2]; 
    json["groupName3"] = groupName[3]; 
    json["groupName4"] = groupName[4]; 
    json["groupName5"] = groupName[5]; 
    json["groupName6"] = groupName[6]; 
    json["groupName7"] = groupName[7]; 
    json["groupName8"] = groupName[8];

    
    json["deviceId1"] = allDeviceA[0].id; 
    json["deviceId2"] = allDeviceA[1].id; 
    json["deviceId3"] = allDeviceA[2].id; 
    json["deviceId4"] = allDeviceA[3].id; 
    json["deviceId5"] = allDeviceA[4].id; 
    json["deviceId6"] = allDeviceA[5].id; 
    json["deviceId7"] = allDeviceA[6].id; 
    json["deviceId8"] = allDeviceA[7].id; 
  
    json["deviceName1"] = allDeviceA[0].deviceName; 
    json["deviceName2"] = allDeviceA[1].deviceName; 
    json["deviceName3"] = allDeviceA[2].deviceName; 
    json["deviceName4"] = allDeviceA[3].deviceName; 
    json["deviceName5"] = allDeviceA[4].deviceName; 
    json["deviceName6"] = allDeviceA[5].deviceName; 
    json["deviceName7"] = allDeviceA[6].deviceName; 
    json["deviceName8"] = allDeviceA[7].deviceName; 
  
    json["deviceGroup1"] = allDeviceA[0].group; 
    json["deviceGroup2"] = allDeviceA[1].group; 
    json["deviceGroup3"] = allDeviceA[2].group; 
    json["deviceGroup4"] = allDeviceA[3].group; 
    json["deviceGroup5"] = allDeviceA[4].group; 
    json["deviceGroup6"] = allDeviceA[5].group; 
    json["deviceGroup7"] = allDeviceA[6].group; 
    json["deviceGroup8"] = allDeviceA[7].group; 
  
    json["deviceType1"] = allDeviceA[0].type; 
    json["deviceType2"] = allDeviceA[1].type; 
    json["deviceType3"] = allDeviceA[2].type; 
    json["deviceType4"] = allDeviceA[3].type; 
    json["deviceType5"] = allDeviceA[4].type; 
    json["deviceType6"] = allDeviceA[5].type; 
    json["deviceType7"] = allDeviceA[6].type; 
    json["deviceType8"] = allDeviceA[7].type;
    
    if(!stateCommand && debugMode) {
      json["secretKey"] = uuidStr;
    }
    
    String config_json;
    json.prettyPrintTo(config_json);
    sendHeaderAccess();
    server.send(200, "text/json", config_json);
        
  } else {
    sendHeaderAccess();
    server.send(404, "text/json", "{\"error\":\"Access denied\"}");
  }
  
}

void sendHeaderAccess() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Max-Age", "10000");
    server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");  
}

void saveConfig(){
  bool configOk = false;
  Serial.println("Receive data");
  if(debugMode && server.args() > 0){
    if(server.hasArg("secretKey")) {
      if(server.arg("secretKey") == uuidStr) {
        Serial.println("secret key is ok");
        String temp;
        if (server.hasArg("gladys_server")) {
          temp = server.arg("gladys_server");
          temp.toCharArray(gladys_server, temp.length()+1); 
          //strcpy(gladys_server, server.arg("gladys_server"));
        }
    
        if (server.hasArg("gladys_port")) {
          temp = server.arg("gladys_port"); 
          temp.toCharArray(gladys_port, temp.length()+1); 
          //strcpy(gladys_port, server.arg("gladys_port"));;
        }
    
        if (server.hasArg("gladys_token")) {
          temp = server.arg("gladys_token"); 
          temp.toCharArray(gladys_token, temp.length()+1);
        }
    
        if (server.hasArg("invertInputDataPortB")) {
          invertInputDataPortB = server.arg("invertInputDataPortB");
        } else {
          invertInputDataPortB = false;
        }
    
        if (server.hasArg("sensorLight")) {
          sensorLight = server.arg("sensorLight");
        } else {
          sensorLight = false;
        }
        
        if (server.hasArg("i2cIntB")) {
          interruptPin = server.arg("i2cIntB").toInt();
        }
        
        if (server.hasArg("i2cSda")) {
          I2C_SDA_PIN = server.arg("i2cSda").toInt();
        }
        if (server.hasArg("i2cScl")) {
          I2C_SCL_PIN = server.arg("i2cScl").toInt();
        }
        if (server.hasArg("i2cAddr")) {
          I2C_ADDR = server.arg("i2cAddr").toInt();
        }
        
        if (server.hasArg("groupName1")) {
          temp = server.arg("groupName1"); 
          temp.toCharArray(groupName[1], temp.length()+1);
          //groupName[0] = server.arg("Group_name1");
        }
        
        if (server.hasArg("groupName2")) {
          temp = server.arg("groupName2"); 
          temp.toCharArray(groupName[2], temp.length()+1);
          //groupName[1] = server.arg("Group_name2");
        }
        if (server.hasArg("groupName3")) {
          temp = server.arg("groupName3"); 
          temp.toCharArray(groupName[3], temp.length()+1);
          //groupName[2] = server.arg("Group_name3");
        }
        if (server.hasArg("groupName4")) {
          temp = server.arg("groupName4"); 
          temp.toCharArray(groupName[4], temp.length()+1);
          //groupName[3] = server.arg("Group_name4");
        }
        if (server.hasArg("groupName5")) {
          temp = server.arg("groupName5"); 
          temp.toCharArray(groupName[5], temp.length()+1);
          //groupName[4] = server.arg("Group_name5");
        }
        if (server.hasArg("groupName6")) {
          temp = server.arg("groupName6"); 
          temp.toCharArray(groupName[6], temp.length()+1);
          //groupName[5] = server.arg("Group_name6");
        }
        if (server.hasArg("groupName7")) {
          temp = server.arg("groupName7"); 
          temp.toCharArray(groupName[7], temp.length()+1);
          //groupName[6] = server.arg("Group_name7");
        }
        if (server.hasArg("groupName8")) {
          temp = server.arg("groupName8"); 
          temp.toCharArray(groupName[8], temp.length()+1);
          //groupName[7] = server.arg("Group_name8");
        }
        
        if (server.hasArg("deviceId1")) {
          allDeviceA[0].id = server.arg("deviceId1").toInt();
        }
        if (server.hasArg("deviceId2")) {
          allDeviceA[1].id = server.arg("deviceId2").toInt();
        }
        if (server.hasArg("deviceId3")) {
          allDeviceA[2].id = server.arg("deviceId3").toInt();
        }
        if (server.hasArg("deviceId4")) {
          allDeviceA[3].id = server.arg("deviceId4").toInt();
        }
        if (server.hasArg("deviceId5")) {
          allDeviceA[4].id = server.arg("deviceId5").toInt();
        }
        if (server.hasArg("deviceId6")) {
          allDeviceA[5].id = server.arg("deviceId6").toInt();
        }
        if (server.hasArg("deviceId7")) {
          allDeviceA[6].id = server.arg("deviceId7").toInt();
        }
        if (server.hasArg("deviceId8")) {
          allDeviceA[7].id = server.arg("deviceId8").toInt();
        }
    
        if (server.hasArg("deviceName1")) {
          temp = server.arg("deviceName1"); 
          temp.toCharArray(allDeviceA[0].deviceName, temp.length()+1);
          //allDeviceA[0].deviceName = server.arg("NAME_device1");
        }
        if (server.hasArg("deviceName2")) {
          temp = server.arg("deviceName2"); 
          temp.toCharArray(allDeviceA[1].deviceName, temp.length()+1);
          //allDeviceA[1].deviceName = server.arg("NAME_device2");
        }
        if (server.hasArg("deviceName3")) {
          temp = server.arg("deviceName3"); 
          temp.toCharArray(allDeviceA[2].deviceName, temp.length()+1);
          //allDeviceA[2].deviceName = server.arg("NAME_device3");
        }
        if (server.hasArg("deviceName4")) {
          temp = server.arg("deviceName4"); 
          temp.toCharArray(allDeviceA[3].deviceName, temp.length()+1);
          //allDeviceA[3].deviceName = server.arg("NAME_device4");
        }
        if (server.hasArg("deviceName5")) {
          temp = server.arg("deviceName5"); 
          temp.toCharArray(allDeviceA[4].deviceName, temp.length()+1);
          //allDeviceA[4].deviceName = server.arg("NAME_device5");
        }
        if (server.hasArg("deviceName6")) {
          temp = server.arg("deviceName6"); 
          temp.toCharArray(allDeviceA[5].deviceName, temp.length()+1);
          //allDeviceA[5].deviceName = server.arg("NAME_device6");
        }
        if (server.hasArg("deviceName7")) {
          temp = server.arg("deviceName7"); 
          temp.toCharArray(allDeviceA[6].deviceName, temp.length()+1);
          //allDeviceA[6].deviceName = server.arg("NAME_device7");
        }
        if (server.hasArg("deviceName8")) {
          temp = server.arg("deviceName8"); 
          temp.toCharArray(allDeviceA[7].deviceName, temp.length()+1);
          //allDeviceA[7].deviceName = server.arg("NAME_device8");
        }
    
        if (server.hasArg("deviceGroup1")) {
          allDeviceA[0].group = server.arg("deviceGroup1").toInt();
        }
        if (server.hasArg("deviceGroup2")) {
          allDeviceA[1].group = server.arg("deviceGroup2").toInt();
        }
        if (server.hasArg("deviceGroup3")) {
          allDeviceA[2].group = server.arg("deviceGroup3").toInt();
        }
        if (server.hasArg("deviceGroup4")) {
          allDeviceA[3].group = server.arg("deviceGroup4").toInt();
        }
        if (server.hasArg("deviceGroup5")) {
          allDeviceA[4].group = server.arg("deviceGroup5").toInt();
        }
        if (server.hasArg("deviceGroup6")) {
          allDeviceA[5].group = server.arg("deviceGroup6").toInt();
        }
        if (server.hasArg("deviceGroup7")) {
          allDeviceA[6].group = server.arg("deviceGroup7").toInt();
        }
        if (server.hasArg("deviceGroup8")) {
          allDeviceA[7].group = server.arg("deviceGroup8").toInt();
        }
    
        if (server.hasArg("deviceType1")) {
          allDeviceA[0].type = server.arg("deviceType1").toInt();
        }
        if (server.hasArg("deviceType2")) {
          allDeviceA[1].type = server.arg("deviceType2").toInt();
        }
        if (server.hasArg("deviceType3")) {
          allDeviceA[2].type = server.arg("deviceType3").toInt();
        }
        if (server.hasArg("deviceType4")) {
          allDeviceA[3].type = server.arg("deviceType4").toInt();
        }
        if (server.hasArg("deviceType5")) {
          allDeviceA[4].type = server.arg("deviceType5").toInt();
        }
        if (server.hasArg("deviceType6")) {
          allDeviceA[5].type = server.arg("deviceType6").toInt();
        }
        if (server.hasArg("deviceType7")) {
          allDeviceA[6].type = server.arg("deviceType7").toInt();
        }
        if (server.hasArg("deviceType8")) {
          allDeviceA[7].type = server.arg("deviceType8").toInt();
        }
        
        if (server.hasArg("delayPush")) {
          delayPush = server.arg("delayPush").toInt();
        }
        writeConfigFile();
        configOk = true;
        initialParam = false;
        blinkLED(1,0,1,0.5);
      }
    }
  }
  sendHeaderAccess();
  if (configOk){  
    server.send(200, "text/json", "{\"state\":\"ok\"}");
  } else {
    server.send(404, "text/json", "{\"error\":\"Access denied\"}");
  }
}

void sendLastValue() {
  byte value_lastPortB = lastPortB;
  if(invertInputDataPortB) {
    value_lastPortB = ~value_lastPortB;
  }
  sendHeaderAccess();
  server.send(200, "text/json", "{\"lastValue\":" + String(value_lastPortB) + "}"); 
}
