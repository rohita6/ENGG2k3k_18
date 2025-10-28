#include <WiFi.h>


const int TRIG_PIN1 = 32;  //Sensor 1
const int ECHO_PIN1 = 33;
const int TRIG_PIN2 = 18;  //Sensor 2
const int ECHO_PIN2 = 19;
;
const int ENA = 2;
const int IN1 = 4;
const int IN2 = 16;
const int output26 = 26;  //RED LED
const int output27 = 27;  //BLUE LED

bool manualOverride = false;
bool manualMode = false;

float distance1 = 0;
float distance2 = 0;

//wifi credentials
const char* ssid = "Group_18_AP";
const char* password = "123";

unsigned long previousTime = 0; //For Void Loop

bool bridgeOpening = false;
unsigned long openStartTime = 0; //For bridge opening

bool bridgeStopping = false;
unsigned long openStopTime = 0; //For bridge stopping

bool bridgeClosing = false;
unsigned long openCloseTime = 0; //For bridge closing


WiFiServer server(80);

String output26State = "CLOSE";

bool shipDetected = false;
// Motor control functions
void openBridge() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 50);  // PWM speed control (0–255)
  bridgeOpening = true;
  bridgeStopping = false;
  bridgeClosing = false;
  openStartTime = millis(); // mark the start
}

void closeBridge() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 50);
  bridgeOpening = false;
  bridgeStopping = false;
  bridgeClosing = true;
  openCloseTime = millis(); // mark the start
}

void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
  bridgeOpening = false;
  bridgeStopping = true;
  bridgeClosing = false;
  openStopTime = millis(); // mark the start
}

void bridgeControl(){
  if (distance1 > 0 && distance1 < 100 || distance2 > 0 && distance2 < 100) { //Open Bridge
    shipDetected = true;
    digitalWrite(output26, HIGH);//Red
    digitalWrite(output27, LOW);
    openBridge();
  } 
  
  else if (bridgeOpening && (millis() - openStartTime >= 3000)) { //Stop the bridge when its been opening for 3 seconds
    stopMotor();
    digitalWrite(output26, HIGH);//Red
    digitalWrite(output27, LOW);
  }
  else if (bridgeStopping && (millis() - openCloseTime >= 3000)) { //Close the bridge when its been stopping for 3 seconds
    closeBridge();
    digitalWrite(output26, HIGH);//Red
    digitalWrite(output27, LOW);
  }
  else if (bridgeClosing && (millis() - openCloseTime >= 3000)) { //Stop the bridge when its been closing for 3 seconds
    stopMotor();
    digitalWrite(output26, LOW);//Red
    digitalWrite(output27, HIGH);
    shipDetected = false;
  }
}

void sensors() {  //SensorCode
  // Clear the trigger PINs
  digitalWrite(TRIG_PIN1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN2, LOW);
  delayMicroseconds(2);

  //Send a 10us pulse to trigger measurement for sensor 1
  digitalWrite(TRIG_PIN1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN1, LOW);

  //Send a 10us pulse to trigger measurement for sensor 2
  digitalWrite(TRIG_PIN2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN2, LOW);
  //Read the echo pulse
  long duration1 = pulseIn(ECHO_PIN1, HIGH);
  long duration2 = pulseIn(ECHO_PIN2, HIGH);

  // Calculate distance in cm (speed of sound: ~343 m/s)
  distance1 = duration1 * 0.0343 / 2;
  distance2 = duration2 * 0.0343 / 2;

  Serial.print("Distance1: ");
  Serial.print(distance1);
  Serial.println("cm");

  Serial.print("Distance2: ");
  Serial.print(distance2);
  Serial.println("cm");
}

String generateControls(int pin, const String& state) {  // for WebPage
  String html = "<p>PIN " + String(pin) + " - Bridge Status " + state + "</p>";
  //State Button
  String nextAction = (state == "CLOSE") ? "OPEN" : "CLOSE";
  String buttonColor = (state == "CLOSE") ? "button" : "button button2";
  html += "<p><a href=\"/" + String(pin) + "/" + nextAction + "\"><button class=\"" + buttonColor + "\">" + nextAction + "</button></a></p>";

  //Manual Override Button
  String modeText = manualMode ? "MANUAL" : "AUTO";
  String nextMode = manualMode ? "AUTO" : "MANUAL";
  String modeColor = manualMode ? "button button2" : "button";
  html += "<p><a href=\"/MODE/" + nextMode + "\"><button class=\"" + modeColor + "\">" + nextMode + " MODE</button></a></p>";

  return html;
}

void sendWebPage(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  client.println(R"rawliteral(
    <!DOCTYPE html><html>
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <link rel="icon" href="data:,">
      <style>
        html { font-family: Helvetica; text-align: center; margin: 0px auto; }
        .button { background-color: #00ff37; border: none; color: white; 
                  padding: 16px 40px; font-size: 30px; cursor: pointer; }
        .button2 { background-color: #ff0019; }
      </style>
    </head>
    <body>
      <h1>ESP32 Web Server</h1>
  )rawliteral");

  // Bridge control button
  client.println(generateControls(output26, output26State));


  // Ship detection info
  if (shipDetected) {
    client.println("<p> Ship Detected <span style=\"color: green;\">True</span></p>");
  } else {
    client.println("<p> Ship Detected <span style=\"color: red;\">False</span></p>");
  }

  client.println(R"rawliteral(
    </body></html>
  )rawliteral");

  client.println();
}

void handleRequest(String& header, WiFiClient& client) {  // for WebPage
  if (header.indexOf("GET /26/OPEN") >= 0) {
    output26State = "OPEN";
    digitalWrite(output26, HIGH);
    digitalWrite(output27, LOW);
    openBridge();
    manualOverride = true;
  } else if (header.indexOf("GET /26/CLOSE") >= 0) {
    output26State = "CLOSE";
    digitalWrite(output26, LOW);
    digitalWrite(output27, HIGH);
    closeBridge();
    manualOverride = true;
  }
  else if (header.indexOf("GET /MODE/MANUAL") >= 0) {
    manualMode = true;
    manualOverride = true;
    Serial.println("Switched to MANUAL mode.");
  } 
  else if (header.indexOf("GET /MODE/AUTO") >= 0) {
    manualMode = false;
    manualOverride = false;
    Serial.println("Switched to AUTO mode.");
  }
  sendWebPage(client);
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN1, OUTPUT);  //Sensors
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);

  //Sets up good LEDs
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  // Sets up Motor Pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  // Begin Wifi Access Point
  Serial.print("Setting Access Point");
  // Username and Password
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  //Make sure motor is stopped initially
  stopMotor();

  server.begin();
}



void loop() {

  WiFiClient client = server.available();
  if (client) {
    String header = "";
    while (client.connected() && client.available()) {
      char c = client.read();
      header += c;
    }
    if (header.length() > 0) handleRequest(header, client);
    client.stop();
  }
  // --- Control logic ---
  if (!manualMode) {
    bridgeControl();
  }
  // Sensor check
  if (millis() - previousTime >= 500) {  // run every 500ms
    previousTime = millis();
    sensors();
  }
}
