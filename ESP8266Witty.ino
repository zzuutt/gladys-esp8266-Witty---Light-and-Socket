/*
   ESP8266 Witty
   http://s.click.aliexpress.com/e/R7aieQ3fe

   + MCP23017
   + 8-channel relay control panel
   + Optocoupler Isolation Voltage Test Board 8 Channel AC 220V

   COMMAND: PUSH & SWITCH

*/
#include <FS.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>  //Ticker Library
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <WiFiManager.h>      //https://github.com/kentaylor/WiFiManager
#include <OneButton.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include "html/i2c_html.h"
#include "html/state_html.h"

Ticker blinker;
Adafruit_MCP23017 mcp;

class deviceMCP {
  public:
    unsigned int pinOut, id, pinIn;
    deviceMCP(unsigned int arg1, unsigned int arg2, unsigned int arg3){
      pinOut = arg1;
      id = arg2;
      pinIn = arg3;  
    }
};

String topPage;
String bottomPage;

//define your default values here, if there are different values in config.json, they are overwritten.
char gladys_server[40];
char gladys_port[6] = "8080";
char gladys_token[34] = "YOUR_GLADYS_TOKEN";
//default custom static IP
char static_ip[16] = "0.0.0.0";
char static_gw[16] = "192.168.0.254";
char static_sn[16] = "255.255.255.0";
char static_dns[16] = "8.8.8.8";
IPAddress _ip,_gw,_sn,_dns;

deviceMCP allDeviceA[8] = {{0,0,8},{1,0,9},{2,0,10},{3,0,11},{4,0,12},{5,0,13},{6,0,14},{7,0,15}};

bool sensorLight = true;

// Constants
const char* CONFIG_FILE = "/config.json";

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
unsigned int I2C_SDA_PIN = 5;
unsigned int I2C_SCL_PIN = 0;
unsigned int I2C_ADDR = 7;
//Delay Push
unsigned int delayPush = 300;
// Interrupts from the MCP will be handled by this PIN
unsigned int interruptPin = 14;
volatile byte interruptCounter = 0;
int numberOfInterrupts = 0;
bool invertInputDataPortB = 0;
byte lastPortB = 0b00000000;

bool sensorState = false;
int lightValue = 0;

bool debugMode = false;
bool espStart = false;

// Indicates whether ESP has WiFi credentials saved from previous session
bool initialConfig = false;

// Delay LongPress (in millisec)
int longPressDelay = 8000;  //8s

// Function Prototypes
bool readConfigFile();
bool writeConfigFile();

OneButton button(BUTTON_PIN_RST, true);
// Set web server port number to 80
ESP8266WebServer server(80);

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

void changeState(int pin) {
  if(pin == 999){   
    digitalWrite(RED_PIN_LED, !(digitalRead(RED_PIN_LED)));  //Invert Current State of LED
    digitalWrite(GREEN_PIN_LED, !(digitalRead(GREEN_PIN_LED)));  //Invert Current State of LED
    digitalWrite(BLUE_PIN_LED, !(digitalRead(BLUE_PIN_LED)));  //Invert Current State of LED
  } else {
    digitalWrite(pin, !(digitalRead(pin)));  //Invert Current State of LED
  }
}

void blinkLED(int c_red = 0, int c_green = 0, int c_blue = 0) {
  blinker.detach();
  black();
  //Initialize Ticker every 0.5s
  if (c_red && c_green && c_blue){
    blinker.attach(0.5, changeState, 999);
  } else if (c_red) {
    blinker.attach(0.5, changeState, RED_PIN_LED);
  } else if (c_green) {
    blinker.attach(0.5, changeState, GREEN_PIN_LED);
  } else if (c_blue) {
    blinker.attach(0.5, changeState, BLUE_PIN_LED);
  }
}

void clearConfig() {
  // is configuration portal requested?
  Serial.println("Configuration portal requested");
  blinkLED(0, 0, 1);
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

  WiFiManagerParameter custom_textServer("<p style=\"border-top: 1px black solid;padding: 5px;margin-top: 10px;\"><b>Server Settings</b></p>");
  // IP Gladys Server - this is a straight forward string parameter
  WiFiManagerParameter p_gladys_server("gladys_server", "IP Gladys Server", gladys_server, 17);

  // PORT Gladys Server - this is a straight forward string parameter
  WiFiManagerParameter p_gladys_port("gladys_port", "PORT Gladys Server", gladys_port, 6);

  // Gladys Token - this is a straight forward string parameter
  WiFiManagerParameter p_gladys_token("gladys_token", "Gladys Token", gladys_token, 34);

  // I2C SCL and SDA parameters are integers so we need to convert them to char array but
  // no other special considerations
  WiFiManagerParameter custom_textI2C("<p style=\"border-top: 1px black solid;padding: 5px;margin-top: 10px;\"><b>I2C Settings</b></p>");
  char convertedValue[4];
  sprintf(convertedValue, "%d", I2C_SDA_PIN);
  WiFiManagerParameter p_I2C_SDA_PIN("I2C_SDA_PIN", "I2C SDA pin", convertedValue, 4);
  sprintf(convertedValue, "%d", I2C_SCL_PIN);
  WiFiManagerParameter p_I2C_SCL_PIN("I2C_SCL_PIN", "I2C SCL pin", convertedValue, 4);
  sprintf(convertedValue, "%d", I2C_ADDR);
  WiFiManagerParameter p_I2C_ADDR("I2C_ADDR", "I2C Addres (0 = 0x20 / 1 = 0x21 ...)", convertedValue, 4);
  sprintf(convertedValue, "%d", interruptPin);
  WiFiManagerParameter p_interruptPin("interruptPin", "Interrupt Pin (INTB)", convertedValue, 4);
  
  //ID device
  WiFiManagerParameter custom_textDevice("<p style=\"border-top: 1px black solid;padding: 5px;margin-top: 10px;\"><b>ID device Settings</b></p>");
  sprintf(convertedValue, "%d", allDeviceA[0].id);
  WiFiManagerParameter p_ID_device1("ID_device1", "ID device - 1", convertedValue, 4);
  sprintf(convertedValue, "%d", allDeviceA[1].id);
  WiFiManagerParameter p_ID_device2("ID_device2", "ID device - 2", convertedValue, 4);
  sprintf(convertedValue, "%d", allDeviceA[2].id);
  WiFiManagerParameter p_ID_device3("ID_device3", "ID device - 3", convertedValue, 4);
  sprintf(convertedValue, "%d", allDeviceA[3].id);
  WiFiManagerParameter p_ID_device4("ID_device4", "ID device - 4", convertedValue, 4);
  sprintf(convertedValue, "%d", allDeviceA[4].id);
  WiFiManagerParameter p_ID_device5("ID_device5", "ID device - 5", convertedValue, 4);
  sprintf(convertedValue, "%d", allDeviceA[5].id);
  WiFiManagerParameter p_ID_device6("ID_device6", "ID device - 6", convertedValue, 4);
  sprintf(convertedValue, "%d", allDeviceA[6].id);
  WiFiManagerParameter p_ID_device7("ID_device7", "ID device - 7", convertedValue, 4);
  sprintf(convertedValue, "%d", allDeviceA[7].id);
  WiFiManagerParameter p_ID_device8("ID_device8", "ID device - 8", convertedValue, 4);

  // LIGHT sensor present or not - bool parameter visualized using checkbox, so couple of things to note
  // - value is always 'T' for true. When the HTML form is submitted this is the value that will be
  //   sent as a parameter. When unchecked, nothing will be sent by the HTML standard.
  // - customhtml must be 'type="checkbox"' for obvious reasons. When the default is checked
  //   append 'checked' too
  // - labelplacement parameter is WFM_LABEL_AFTER for checkboxes as label has to be placed after the input field
  WiFiManagerParameter custom_textLight("<p style=\"border-top: 1px black solid;padding: 5px;margin-top: 10px;\"><b>Light Settings</b></p>");
  char customhtmllight[] = "type='checkbox'";
  if (sensorLight) {
    strcat(customhtmllight, " checked");
  }
  WiFiManagerParameter p_sensorLight("sensorlight", "Disable LED indicator the night", "T", 2, customhtmllight, WFM_LABEL_AFTER);

  // Invert input data PORTB
  WiFiManagerParameter custom_textInputDataPortB("<p style=\"border-top: 1px black solid;padding: 5px;margin-top: 10px;\"><b>Input data PortB</b></p>");
  WiFiManagerParameter p_invertInputDataPortB("invertInputDataPortB", "invert data PORTB", "T", 2, "type='checkbox' checked", WFM_LABEL_AFTER);

  char convertedValue2[5];
  WiFiManagerParameter custom_textDelayPush("<p style=\"border-top: 1px black solid;padding: 5px;margin-top: 10px;\"><b>Command PUSH</b></p>");
  sprintf(convertedValue2, "%d", delayPush);
  WiFiManagerParameter p_delayPush("delayPush", "Pressure duration in milliseconds (0 to 9999)", convertedValue2, 5);

  // Just a quick hint
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
  
  wifiManager.addParameter(&custom_textServer);
  wifiManager.addParameter(&p_gladys_server);
  wifiManager.addParameter(&p_gladys_port);
  wifiManager.addParameter(&p_gladys_token);

  wifiManager.addParameter(&custom_textI2C);
  wifiManager.addParameter(&p_I2C_SDA_PIN);
  wifiManager.addParameter(&p_I2C_SCL_PIN);
  wifiManager.addParameter(&p_I2C_ADDR);
  wifiManager.addParameter(&p_interruptPin);

  wifiManager.addParameter(&custom_textDelayPush);
  wifiManager.addParameter(&p_delayPush);
 
  wifiManager.addParameter(&custom_textDevice);
  wifiManager.addParameter(&p_ID_device1);
  wifiManager.addParameter(&p_ID_device2);
  wifiManager.addParameter(&p_ID_device3);
  wifiManager.addParameter(&p_ID_device4);
  wifiManager.addParameter(&p_ID_device5);
  wifiManager.addParameter(&p_ID_device6);
  wifiManager.addParameter(&p_ID_device7);
  wifiManager.addParameter(&p_ID_device8);

  wifiManager.addParameter(&custom_textLight);
  wifiManager.addParameter(&p_sensorLight);

  wifiManager.addParameter(&custom_textInputDataPortB);
  wifiManager.addParameter(&p_invertInputDataPortB);
  
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
    blinkLED(0,0,1);

    // Getting posted form values and overriding local variables parameters
    // Config file is written regardless the connection state
    strcpy(static_ip, p_static_ip.getValue());
    strcpy(static_gw, p_static_gw.getValue());
    strcpy(static_sn, p_static_sn.getValue());
    strcpy(static_dns, p_static_dns.getValue());

    strcpy(gladys_server, p_gladys_server.getValue());
    strcpy(gladys_port, p_gladys_port.getValue());
    strcpy(gladys_token, p_gladys_token.getValue());
    sensorLight = (strncmp(p_sensorLight.getValue(), "T", 1) == 0);
    I2C_SDA_PIN = atoi(p_I2C_SDA_PIN.getValue());
    I2C_SCL_PIN = atoi(p_I2C_SCL_PIN.getValue());
    I2C_ADDR = atoi(p_I2C_ADDR.getValue());
    interruptPin = atoi(p_interruptPin.getValue());
    invertInputDataPortB = (strncmp(p_invertInputDataPortB.getValue(), "T", 1) == 0);
    allDeviceA[0].id = atoi(p_ID_device1.getValue());
    allDeviceA[1].id = atoi(p_ID_device2.getValue());
    allDeviceA[2].id = atoi(p_ID_device3.getValue());
    allDeviceA[3].id = atoi(p_ID_device4.getValue());
    allDeviceA[4].id = atoi(p_ID_device5.getValue());
    allDeviceA[5].id = atoi(p_ID_device6.getValue());
    allDeviceA[6].id = atoi(p_ID_device7.getValue());
    allDeviceA[7].id = atoi(p_ID_device8.getValue());
    delayPush = atoi(p_delayPush.getValue());

    blinkLED(0, 0, 0);
    black();
    // Writing JSON config file to flash for next boot
    Serial.println("Save data....");
    if (writeConfigFile()) {
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
  blinkLED(1,0,0);
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

  blinkLED(0, 1, 0);

  button.attachDoubleClick(displayData);            // link the function to be called on a doubleclick event.
  button.attachClick(debugState);                  // link the function to be called on a singleclick event.
  button.setPressTicks(longPressDelay);
  button.attachLongPressStart(resetConfig);            // link the function to be called on a longpress event.

  // Mount the filesystem
  bool result = SPIFFS.begin();
  Serial.println("SPIFFS opened: " + result);

  if (!readConfigFile()) {
    Serial.println("Failed to read configuration file, using default values");
    initialConfig = true;
  }

  WiFi.printDiag(Serial); //Remove this line if you do not want to see WiFi password printed
  if (WiFi.SSID() == "") {
    Serial.println("We haven't got any access point credentials, so get them now");
    initialConfig = true;
  } else {
    if (!initialConfig) {
      if(static_ip != "" && static_ip != "0.0.0.0") {
        //set static ip
        _ip.fromString(static_ip);
        _gw.fromString(static_gw);
        _sn.fromString(static_sn);
        _dns.fromString(static_dns);

        WiFi.config(_ip, _dns, _gw, _sn);
        Serial.println("Init static IP");
      }
      
      blinkLED(0, 1, 0); // Turn LED off as we are not in configuration mode.
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
    blinkLED(1, 0, 0);
    Serial.println("Failed to connect, finishing setup anyway");
  } else {

    //Define I2C pins
    Wire.begin(I2C_SDA_PIN,I2C_SCL_PIN);
    // MCP23017 address
    mcp.begin(I2C_ADDR);

    for (int i = 0; i < 8; i++){
      // MCP23017 port A -> Output
      mcp.pinMode(allDeviceA[i].pinOut, OUTPUT);
      // MCP23017 port B -> Input
      mcp.pinMode(allDeviceA[i].pinIn, INPUT);
      mcp.pullUp(allDeviceA[i].pinIn, HIGH);
    }

    // MCP23017 Interrupt test
    pinMode(interruptPin, INPUT);
    for (int i = 0; i < 8; i++){
      mcp.setupInterruptPin(allDeviceA[i].pinIn,CHANGE);  
    }

    // enable interrupts before going to sleep/wait
    // And we setup a callback for the arduino INT handler.
    attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, CHANGE);
                
    blinkLED(0, 0, 0);
    white();
    Serial.print("Local ip: ");
    Serial.println(WiFi.localIP());
    //all the sites which will be available
    server.on( "/", getHeatindex );
    server.on( "/scanI2C", scanPorts );
    server.on( "/checkport", checkPort );
    server.on( "/readPort", ajax_readPortB );
    server.begin();
    espStart = 1;
  }
}

void handleInterrupt() {
  interruptCounter++;
  if(debugMode){
    Serial.println("An interrupt has occurred. Counter: " + String(interruptCounter));
  }
}

void loop() {
  // is configuration portal requested?
  if (initialConfig) {
    clearConfig();
  }
  if (espStart){
    readPortB();
    espStart = 0;
  }
  //process all the requests for the Webserver
  server.handleClient();    
  
  // Read state button
  button.tick();
  delay(10);
  
  bool sensor = sensorStateLCR();
 
  if(sensor && !debugMode){
    white();
  } else {
    black();
  }
    
  if(interruptCounter > 0){
    interruptCounter--;
    numberOfInterrupts++;
    if(debugMode){
      Serial.println("An interrupt has occurred. Total: " + String(numberOfInterrupts));
      Serial.println("-------- DEBUG --------");
      Serial.println("--Device state change--");
    }

    readPortB();
    
    if(debugMode){
      Serial.println("-------- ----- --------");
    }
  }

}

void readPortB(){
  //read state all port B
  byte statePortB = mcp.readGPIO(1);
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
        sendStateToGladys(stateBit, allDeviceA[i].id); 
      }
    }
    lastPortB = statePortB;
  }  
}

bool sensorStateLCR(){
  if(sensorLight){
    lightValue = analogRead(LIGHT_PIN);
    if(lightValue > 300){
      sensorState = 1;
    } else {
      sensorState = 0;
    }
  } else {
    sensorState = 1;
  }

    return sensorState;
}

int sendStateToGladys (bool realState, int deviceTypeId){
  HTTPClient http;
  String getData, link;
  bool sensor = sensorStateLCR();
  
  if(sensor && !debugMode){
    blinkLED(1, 1, 1);
  } else {
    black();
  }
  
  getData = "?token=" + String(gladys_token) + "&devicetype=" + String(deviceTypeId) + "&value=" + String(realState);
  link = "http://" + String(gladys_server) + ":" + String(gladys_port) + "/devicestate/create" + getData;
  http.begin(link);
  int httpCode = http.GET();
  String payload = http.getString();
  if(debugMode){
    Serial.println("Link: " + link);
    Serial.println("http code: " + httpCode);
    Serial.println("response: " + payload + "\n");
  }

  http.end();
  
  if(!debugMode){
    blinkLED(0,0,0);
  } else {
    blinkLED(0,1,0);
  }
  
  return httpCode;
}

bool readConfigFile() {
  // this opens the config file in read-mode
  File f = SPIFFS.open(CONFIG_FILE, "r");

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
    
    if (json.containsKey("delayPush")) {
      delayPush = json["delayPush"];
    }
  }
  Serial.println("\nConfig file was successfully parsed");
  return true;
}

bool writeConfigFile() {
  Serial.println("Saving config file");

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();

  // JSONify local configuration parameters
  json["static_ip"] = static_ip;
  json["static_gw"] = static_gw;
  json["static_sn"] = static_sn;
  json["static_dns"] = static_dns;
    
  json["gladys_server"] = gladys_server;
  json["gladys_port"] = gladys_port;
  json["gladys_token"] = gladys_token;
  json["invertInputDataPortB"] = invertInputDataPortB;
  json["sensorLight"] = sensorLight;
  json["interruptPin"] = interruptPin;
  json["I2C_SDA_PIN"] = I2C_SDA_PIN;
  json["I2C_SCL_PIN"] = I2C_SCL_PIN;
  json["I2C_ADDR"] = I2C_ADDR;
  
  json["ID_device1"] = allDeviceA[0].id;
  json["ID_device2"] = allDeviceA[1].id;
  json["ID_device3"] = allDeviceA[2].id;
  json["ID_device4"] = allDeviceA[3].id;
  json["ID_device5"] = allDeviceA[4].id;
  json["ID_device6"] = allDeviceA[5].id;
  json["ID_device7"] = allDeviceA[6].id;
  json["ID_device8"] = allDeviceA[7].id;

  json["delayPush"] = delayPush;

  // Open file for writing
  File f = SPIFFS.open(CONFIG_FILE, "w");
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
      Serial.println("   " + String(i+1) + ":  -Pin In: " + String(allDeviceA[i].pinIn + 1) + "  -ID: " + allDeviceA[i].id + "  -Pin Out: " + String(allDeviceA[i].pinOut + 1));
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
    blinkLED(0,0,0);
  } else {
    blinkLED(0,1,0);
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
    if(deviceID == allDeviceA[i].id){
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
  byte statePortB = mcp.readGPIO(1);
  realState = bitRead(statePortB, (allDeviceA[deviceConcerned].pinIn - 7));
  if(invertInputDataPortB) {
    realState = !realState;
  }
  if(debugMode){
    Serial.println("Order " + String(cmdExec) + " change state pinOut: " + String(allDeviceA[deviceConcerned].pinOut) + " pinIn: " + String(allDeviceA[deviceConcerned].pinIn) + " realstate: " + String(realState) + " currentState: " + String(currentState));
  }
  if (realState == currentState){
    if (cmdExec == "PUSH") {
        //exec push command
        return pushCmd(deviceConcerned);
    } else if(cmdExec == "SWITCH") {
        //exec switch command
        return switchCmd(deviceConcerned, !currentState);
    }
  } else if(debugMode){
    Serial.println("The real state does not match the current state");
  }
  return 0;
}

bool pushCmd(int deviceConcerned){
  if(debugMode){
    Serial.println("Order PUSH change state pin: " + String(allDeviceA[deviceConcerned].pinOut) + " state: 1");
  }
  mcp.digitalWrite(allDeviceA[deviceConcerned].pinOut, 1);
  delay(delayPush);
  if(debugMode){
    Serial.println("Order PUSH change state pin: " + String(allDeviceA[deviceConcerned].pinOut) + " state: 0");
  }
  mcp.digitalWrite(allDeviceA[deviceConcerned].pinOut, 0);
  return 1;
}

bool switchCmd(int deviceConcerned, bool state){
  if(debugMode){
    Serial.println("Order SWITCH change state pin: " + String(allDeviceA[deviceConcerned].pinOut) + " state: " + String(state));
  }
  mcp.digitalWrite(allDeviceA[deviceConcerned].pinOut, state);
  return 1;
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
      if(!debugMode){
        changeState(deviceConcerned, cmd, state);
      } else {
        Serial.println("Order received:");
        Serial.println("deviceID: " + String(deviceConcerned) + " cmd: " + cmd + " state: " + String(state));
        changeState(deviceConcerned, cmd, state); 
      }
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
    detachInterrupt(interruptPin);
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
    String pageFinal = I2C_page;
    pageFinal.replace("%%DATA%%", sendWebPage);
    server.send(200, "text/html", pageFinal);
    Wire.begin(I2C_SDA_PIN,I2C_SCL_PIN);
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
  //delay(1000);           // wait 1 seconds for next scan, did not find it necessary
  return sendWebPage;
}

void checkPort(){
  if(debugMode){
    String sendWebPage = "<table><tbody>";
    //read state all port B
    byte statePortB = mcp.readGPIO(1);
    //search for the port that has changed
    for (int i=0; i<8; i++){
      byte stateBit = bitRead(statePortB, i);
      if(invertInputDataPortB) {
        stateBit = !stateBit;
      }
      sendWebPage += "<tr><td> Port: " + String(i) + " ID: " + String(allDeviceA[i].id) + "</td><td>" + stateBit + "</td></tr>";
    }
    sendWebPage += "</tbody></table>";
    String pageFinal = STATE_page;
    pageFinal.replace("%%DATA%%", sendWebPage);
    server.send(200, "text/html", pageFinal);   
  } else {
    server.send(404, "text/plain", "404: Not found");
  }
}

void ajax_readPortB(){
  if(debugMode){
    String sendWebPage = "<table><tbody>";
    //read state all port B
    byte statePortB = mcp.readGPIO(1);
    //search for the port that has changed
    for (int i=0; i<8; i++){
      byte stateBit = bitRead(statePortB, i);
      if(invertInputDataPortB) {
        stateBit = !stateBit;
      }
      sendWebPage += "<tr><td> Port: " + String(i) + " ID: " + String(allDeviceA[i].id) + "</td><td>" + stateBit + "</td></tr>";
    }
    sendWebPage += "</tbody></table>";
    server.send(200, "text/html", sendWebPage);   
  } else {
    server.send(404, "text/plain", "404: Not found");
  }
}
