#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <string>

#include "config.h"
#include "utils.hpp"
#include "parameters.hpp"

ESP8266WebServer server(80);

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

  connectServer();

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
    changeLEDPattern();
  }

  delay(global_delay / 2);
}

///// PROGRAM LOGIC
void changeLEDPattern() {
  if (done_pattern) {
    getPatternParams();
    delay(10);
  }

  if (pattern == 1) {     // breath effect
    Serial.println("PATTERN == 1");
    adjustBrightness();
    delay(global_delay / 2);
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
      global_red = item_step["red"];
      global_green = item_step["green"];
      global_blue = item_step["blue"];
      pattern = item_step["pattern"];
      
      transition_in_delay = limit((((float) N)/ (float) item_step["transition_in_delay"]) * global_delay);
      static_delay = limit((((float) N)/ (float) item_step["static_delay"]) * global_delay);
      transition_off_delay = limit((((float) N)/ (float) item_step["transition_off_delay"]) * global_delay);
    }
    local_counter += 1;
  }

  print("Red: ", String(global_red));
  print("Green: ", String(global_green));
  print("Blue: ", String(global_blue));
  print("Pattern: ", String(pattern));
  print("Transition in delay: ", String(transition_in_delay));
  print("Static delay: ", String(static_delay));
  print("Transition off delay: ", String(transition_off_delay));
  delay(10);

  step += 1;
  if (step >= len_pattern_list) {
    step = 0;
  }
  print("Step: ", String(step));
  delay(20);

  done_pattern = false;
  ascending = true;
  done_delay = false;
  Serial.println("DONE PATTERN == FALSE");
}

void adjustBrightness() {
  if (brightness >= 255) {
    ascending = false;
  }

  if (ascending) {
    brightness += transition_in_delay;
    return;
  }

  sum_delay += static_delay;
  if (sum_delay > 255) {
    sum_delay = 0;
    done_delay = true;
  }

  if (!done_delay) {
    return;
  }

  brightness -= transition_off_delay;

  if (brightness <= 0) {
    brightness = 0;
    done_pattern = true;
  }
  print("Brightness: ", String(brightness));
}

void changeLED(int red, int green, int blue, int bright) {
  analogWrite(PIN_RED, getColor(red));
  analogWrite(PIN_GREEN, getColor(green));
  analogWrite(PIN_BLUE, getColor(blue));
  analogWrite(PIN_BRIGHT, bright);
}

int getIntParameters(String param) {
  const char* req_param = server.arg(String(param)).c_str();
  int int_param = atoi(req_param);

  return int_param;
}

int limit(int value) {
  if (value > 255) {
    return 255;
  }

  if (value < 0) {
    return 0;
  }

  return value;
}

void setVariables(bool onValue) {
  ON = onValue;
  done_pattern = true;
  ascending = true;
  brightness = 0;
  step = 0;
  static_delay = 0;
  transition_in_delay = 0;
  transition_off_delay = 0;
  sum_delay = 0;
  done_delay = false;
}