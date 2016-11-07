 #include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

extern "C" {
  #include "user_interface.h"
}

const char* ssid = "TP-LINK_19EC";
const char* password = "05027767";

ESP8266WebServer server(80);

const int led = 13;
int count = 0;
float velocity = 0;
bool play = false;
String cashback = "", mode_ = "";


void handleRoot() {
  String t;
  if(play)
    t = "play," + String(count) + "," + String(velocity) + "," + cashback + "," + mode_;
  else
    t = "stop";
  //Serial.println(t);
  server.send(200, "text/plain", t);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

const int gerkonPin = 2;

void stateChanged() {
  if(play) {
    count++;
    velocity += 1.0f;
    //Serial.print("G");
  }
}

void setup(void){
  pinMode(gerkonPin, INPUT);
  attachInterrupt(gerkonPin, stateChanged, CHANGE);// RISING
  //os_timer_setfn(&myTimer, timerCallback, NULL);
  //os_timer_arm(&myTimer, 100, true);
  
  //pinMode(led, OUTPUT);
  //digitalWrite(led, 0);
  Serial.begin(115200);
  wifi_set_phy_mode(PHY_MODE_11B);

  if(true)
  {
    WiFi.mode(WIFI_STA);
    WiFi.softAPdisconnect(true);
    WiFi.begin(ssid, password);
    Serial.println("");
  
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("esp8266")) {
      Serial.println("MDNS responder started");
    }
  }
  else
  {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("espbi", "12345678bibi");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP: ");
    Serial.println(myIP);
  }

  server.on("/", handleRoot);

  server.on("/gui", [](){
    String t;
    if(play)
      t = "play," + String(count) + "," + String(velocity) + "," + cashback + "," + mode_;
    else
      t = "stop";
    //Serial.println(t);
    t = "<!DOCTYPE html><html><header><meta http-equiv='refresh' content='1'></header><body><H1>" + t + "</H1>";
    t += "<form action='/play' method='POST'><input type='text' name='cashback'><input type='submit' value='play'></form>";
    t += "<form action='/stop' method='POST'><input type='submit' value='stop'></form></body>";
    server.send(200, "text/html", t);
  });
  
  server.on("/play", [](){
    if(server.method() == HTTP_POST)
    {
      play = true;
      //velocity = 0.0;
      count = 0;
      for (uint8_t i = 0; i < server.args(); i++)
        if(server.argName(i) == "cashback") cashback = server.arg(i);
        else if(server.argName(i) == "mode") mode_ = server.arg(i);
      
      server.send(200, "text/plain", "OK");
    }
  });
  
  server.on("/stop", [](){
    if(server.method() == HTTP_POST)
    {
      play = false;
      //velocity = 0.0;
      count = 0;
      cashback = "0";
      server.send(200, "text/plain", "OK");
    }
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started"); 
}

unsigned long previousMillis = 0;
const long interval = 50;

void loop(void){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;   
    velocity *= 0.9;
  }
  server.handleClient();
}

