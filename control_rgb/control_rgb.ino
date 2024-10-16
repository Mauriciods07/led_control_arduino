#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <string>

#include "config.h"

const int PIN_BRIGHT = 15;
const int PIN_RED = 14;
const int PIN_GREEN = 12;
const int PIN_BLUE = 13;

ESP8266WebServer server(80);

// handler variables
bool ON = false;
int pattern = 0;
int len_pattern_list = 0;
String jsonString;

int step = 0;
int brightness = 0;
bool ascending = true;
bool done_pattern = true;

// pattern variables
int global_red = 0;
int global_green = 0;
int global_blue = 0;
int global_delay = 0;

////// HANDLERS
void getStatus() {
  Serial.println("Obteniendo status");

  server.send(200, "application/json", "Success");
}

void turnLightOn() {
  setVariables(false);

  Serial.println("Turning light on");

  int int_red = getIntParameters("red");
  int int_green = getIntParameters("green");
  int int_blue = getIntParameters("blue");
  int int_intensity = getIntParameters("intensity");
  
  print("Red int: ", String(int_red));
  print("Green int: ", String(int_green));
  print("Blue int: ", String(int_blue));
  print("Intensity int: ", String(int_intensity));

  changeLED(int_red, int_green, int_blue, int_intensity);

  server.send(200, "application/json", "Success");
}

void turnOff() {
  setVariables(false);

  Serial.println("Apagando LED");
  
  changeLED(0, 0, 0, 0);

  server.send(200, "application/json", "Success");
}

void makePattern() {
  setVariables(true);

  print("Parameters: ", server.arg("plain"));
  
  StaticJsonDocument<300> JSONData;
  jsonString = server.arg("plain");
  DeserializationError error = deserializeJson(JSONData, jsonString);
  
  if (error) {
    Serial.print("deserializeJson failed: ");
    Serial.println(error.f_str());
    server.send(500, "application/json", "Error in parsing");
    return;
  }

  JsonArray pattern_list = JSONData["data"];
  Serial.println(String(JSONData["data"]));

  len_pattern_list = len(pattern_list);
  print("Counter: ", String(len_pattern_list));

  Serial.println("Encendiendo LED de colores");
  server.send(200, "application/json", "Success");
}

void handleNotFound() {
  server.send(404, "application/json", "Not found");
}

///// SETUP
void setup() {
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_BRIGHT, OUTPUT);

  Serial.begin(9600);
  delay(10);

  // configuration for the GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2,LOW);

  changeLED(0, 0, 0, 0);
  
  Serial.println();
  Serial.println();
  Serial.print("Conectandose a la red: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Conexion establecida");

  Serial.println("IP del servicio: ");
  Serial.println(WiFi.localIP());

  // endpoints
  server.on("/info", HTTP_GET, getStatus);
  server.on("/staticLight", HTTP_GET, turnLightOn);
  server.on("/turnOff", HTTP_GET, turnOff);
  server.on("/lightPattern", HTTP_POST, makePattern);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  server.handleClient();

  if (ON && len_pattern_list != 0) {
    Serial.println("ENTER CONDITION PATTERN");
    changeLEDPattern();
    delay(10);
  }

  delay(10);
}

///// PROGRAM LOGIC
void changeLEDPattern() {
  if (done_pattern) {
    Serial.println("DONE PATTERN");
    getPatternParams();
    delay(10);
  }

  if (pattern == 1) {     // breath effect
    Serial.println("PATTERN == 1");
    adjustBrightness();
    delay(10);
  }

  changeLED(global_red, global_green, global_blue, brightness);
}

void getPatternParams() {
  print("Pattern: ", String(step));
  Serial.println(jsonString);

  StaticJsonDocument<300> JSONData;
  DeserializationError error = deserializeJson(JSONData, jsonString);
  
  if (error) {
    Serial.print("deserializeJson failed: ");
    Serial.println(error.f_str());
    server.send(500, "application/json", "Error in parsing");
    return;
  }
  Serial.println("Pattern OK");

  JsonArray pattern_list = JSONData["data"];
  print("PATTERN LIST: ", String(pattern_list));
  
  int local_counter = 0;

  for (JsonVariant item_step: pattern_list) {
    if (local_counter == step) {
      Serial.print(local_counter);
      Serial.print(" = ");
      Serial.println(step);

      global_red = item_step["red"];
      global_green = item_step["green"];
      global_blue = item_step["blue"];
      global_delay = item_step["delay"];
      pattern = item_step["pattern"];
    }
    local_counter += 1;
  }

  print("Red: ", String(global_red));
  print(", Green: ", String(global_green));
  print(", Blue: ", String(global_blue));
  print(", Delay: ", String(global_delay));
  print(", Pattern: ", String(pattern));
  delay(10);


  step += 1;
  if (step >= len_pattern_list) {
    step = 0;
  }
  print("Step: ", String(step));
  delay(20);

  done_pattern = false;
  ascending = true;
  Serial.println("DONE PATTERN == FALSE");
}

void adjustBrightness() {
  if (brightness >= 255) {
    ascending = false;
  }
  delay(10);

  if (ascending) {
    brightness += 5;
    return;
  }
  delay(10);

  brightness -= 5;

  if (brightness <= 0) {
    brightness = 0;
    done_pattern = true;
  }
  print("Brightness: ", brightness);
  delay(10);
}

void changeLED(int red, int green, int blue, int bright) {
  analogWrite(PIN_RED, getColor(red));
  analogWrite(PIN_GREEN, getColor(green));
  analogWrite(PIN_BLUE, getColor(blue));
  analogWrite(PIN_BRIGHT, bright);
}

int getColor(int color) {
  return 255 - color;
}

int len(JsonArray jsonArray) {
  int count = 0;

  for (JsonVariant item: jsonArray) {
    count += 1;
  }

  return count;
}

void setVariables(bool onValue) {
  ON = onValue;
  done_pattern = true;
  ascending = true;
  brightness = 0;
  step = 0;
}

int getIntParameters(String param) {
  const char* req_param = server.arg(String(param)).c_str();
  int int_param = atoi(req_param);

  return int_param;
}

void print(String text, String param) {
  Serial.print(text);
  Serial.println(param);
}