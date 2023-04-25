/*********
Robert Sawyer, aka TrueGeek
*********/

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "LittleFS.h"

const char* ssid = "Integro";
const char* password = "Godisgood4ever";
int bellPower = LOW, onTime, offTime, Cycles, iterations;
unsigned long onEnd, offEnd;

// Hardware interrupt to handle the signal form the door switch.
ICACHE_RAM_ATTR void bellOn() {
  // disable interrupts, so that opening and closing the door quickly does not burn out the bell with start/stop Cycles.
  noInterrupts();
  Serial.println("Interupted!!");
  iterations = Cycles;
  bellPower = HIGH;
  // start the bell
  digitalWrite(16, bellPower);
}
// Set web server port number to 80
ESP8266WebServer server(80);

// Variable to store the HTTP request
String header;

// Assign GPIO pins
const int output = 16;
const int input = 15;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output, OUTPUT);
  pinMode(input, INPUT);
  attachInterrupt(digitalPinToInterrupt(input), bellOn, RISING);
  // Make sure the bell is off
  digitalWrite(output, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.println();  // move to the next line after initial garbage
  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();

  server.on("/", handleRoot);
  server.on("/On", handleOn);
  server.on("/Off", handleOff);
  server.on("/Rings", handleRings);
  server.on("/Submitted", handleSubmitted);
  server.onNotFound(handleNotFound);
  server.on("/DIAG", handleDIAG);
  LittleFS.begin();
  //LittleFS.format();

  File written = LittleFS.open("/variables.bin", "r");
  if (written) {
    int it = 0;
    char vars[] = {};
    while (written.available()) {
      char c = written.read();
      vars[it] = (c);
      it++;
    }

    char* token = strtok(vars, ",");
    String on = String(token);
    onTime = on.toInt();

    String values[] = {};
    Serial.println("Before");
    for (int i = 0; i <= 2; i++) {
      Serial.println("inside");
      token = strtok(NULL, ",");
      Serial.println(token);
      values[i] = token;
    }

    String second = String(values[0]);
    String third = String(values[1]);
    offTime = second.toInt();
    Cycles = third.toInt();
    written.close();

  } else {
    Serial.println("No File");
    onTime = 3000;
    offTime = 3000;
    Cycles = 2;
  }
}

void handleNotFound() {
  server.send(404, "text/html", "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"icon\" href=\"data:,\"><style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}.button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}</style></head><body><h1>Page Not Found</h1></br><a href=\"/\"><button class=\"button\">Go Home</button></a></p></body></html>");
}

void handleRoot() {
  String On = String(onTime / 1000);
  String Off = String(offTime / 1000);
  String Rings = String(Cycles);

  server.send(200, "text/html", "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"icon\" href=\"data:,\"><style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}.button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}</style></head><body><h1>Bell Configuration:</h1><h2>Current Settings:</h2><p>Bell on time: " + On + " seconds. </p></br><p>Bell off time: " + Off + " seconds. </p></br><p>Number of rings: " + Rings + " seconds. </p></br><p></br><a href=\"/On\"><button class=\"button\">Change Settings</button></a></p></body></html>");
}

void handleOn() {
  server.send(200, "text/html", "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"icon\" href=\"data:,\"><style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}.button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}</style></head><body><h1>Bell Configuration:</h1><label for='SecondsOn'>Number of seconds for the bell to be on:</label></br></br><input type='number' id='Seconds' name='SecondsOn' maxlength='1' <p></br></br><button class = \"button\" onclick=\"window.location.href = `Off?param=${document.getElementById('Seconds').value}`;\">Next</button></p></body></html>");
}
// document.getElementById('Seconds').value)
void handleOff() {
  // Check if the query parameter exists
  if (!server.hasArg("param")) {
    server.send(400, "text/plain", "Bad Request - Missing parameter");
    return;
  }

  // Parse the query parameter from the URL
  String param_value_str = server.arg("param");

  // Convert the query parameter to an integer
  onTime = param_value_str.toInt() * 1000;

  server.send(200, "text/html", "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"icon\" href=\"data:,\"><style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}.button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}</style></head><body><h1>Bell Configuration:</h1><label for='SecondsOff'>Number of seconds for the bell to be off:</label></br></br><input type='number' id='Seconds' name='SecondsOff' maxlength='1' <p></br></br><button class = \"button\" onclick=\"window.location.href = `Rings?param=${document.getElementById('Seconds').value}`;\">Next</button></body></html>");
}

void handleRings() {
  // Check if the query parameter exists
  if (!server.hasArg("param")) {
    server.send(400, "text/plain", "Bad Request - Missing parameter");
    return;
  }

  // Parse the query parameter from the URL
  String param_value_str = server.arg("param");

  // Convert the query parameter to an integer
  offTime = param_value_str.toInt() * 1000;

  server.send(200, "text.html", "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"icon\" href=\"data:,\"><style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}.button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}</style></head><body><h1>Bell Configuration:</h1><label for='Rings'>Number of times for the bell to ring:</label></br></br><input type='number' id='Rings' name='Rings' maxlength='1' <p></br></br><button class = \"button\" onclick=\"window.location.href = `Submitted?param=${document.getElementById('Rings').value}`;\">Next</button></p></body></html>");
}

void handleSubmitted() {
  // Check if the query parameter exists
  if (!server.hasArg("param")) {
    server.send(400, "text/plain", "Bad Request - Missing parameter");
    return;
  }

  // Parse the query parameter from the URL
  String param_value_str = server.arg("param");

  // Convert the query parameter to an integer
  Cycles = param_value_str.toInt();

  String On = String(onTime / 1000);
  String Off = String(offTime / 1000);
  String Rings = String(Cycles);

  if (LittleFS.exists("/variables.bin")) {
    Serial.print("Removing ");
    LittleFS.remove("/variables.bin");
  }

  // Write variable to text file for persistance across reboots.
  File text = LittleFS.open("/variables.bin", "w");
  if (!text) {
    Serial.println("no file");
  }
  String variables = String(onTime);
  variables.concat(",");
  variables.concat(String(offTime));
  variables.concat(",");
  variables.concat(Cycles);
  variables.concat(",");

  Serial.print(variables);
  text.print(variables);

  text.close();

  server.send(200, "text/html", "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><link rel=\"icon\" href=\"data:,\"><style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}.button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}</style></head><body><h1>Submitted!</h1><h2>New Settings:</h2><p>Bell on time: " + On + " seconds. </p></br><p>Bell off time: " + Off + " seconds. </p></br><p>Number of rings: " + Rings + " seconds. </p></br><p><a href=\"/\"><button class=\"button\">Go Home</button></a></p></body></html>");
}

void handleDIAG() {
  server.send(200, "text/html", "<!DOCTYPE html><html><head><h1>DIAG RESULTS</h1><body><p>You probably do not actually want to see under this hood, but if you really think that you do, you can check <a href = \"https://github.com/rassawyer/Bell_Control\">GitHub Repo</a></body></html>");
}

//=============================================================================

void loop() {
  server.handleClient();

  // If the bell is on, and it has been on for the set length of time, turn it off,
  // and set the time for it to turn back on.
  if (bellPower == HIGH && millis() >= onEnd) {
    bellPower = LOW;
    offEnd = millis() + offTime;
  }

  // If the bell is off AND has been off for the set amount of time, turn it on
  // and set the time for it to turn off, as well as decrement the number of times that it should ring.
  if (bellPower == LOW && millis() >= offEnd && iterations >= 1) {
    iterations--;
    bellPower = HIGH;
    onEnd = millis() + onTime;
  }

  // If the bell is done ringing, re-enable interrupts, to catch the next time the door opens.
  if (iterations == 1) {
    interrupts();
  }

  // set the bell to the current value of bellPower.
  digitalWrite(output, bellPower);
}
