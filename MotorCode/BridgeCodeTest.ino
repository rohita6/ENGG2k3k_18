#include <WiFi.h>

int count = 0;

const int TRIG_PIN1 = 32;  // Sensor 1
const int ECHO_PIN1 = 33;
const int TRIG_PIN2 = 18;  // Sensor 2
const int ECHO_PIN2 = 19;

const int ENA = 2;
const int IN1 = 4;
const int IN2 = 16;
const int output26 = 26;  // RED LED
const int output27 = 27;  // BLUE LED

bool manualOverride = false;
bool manualMode = false;

float distance1 = 0;
float distance2 = 0;

// --- NEW --- distance limits for stopping the motor
const float MAX_LIFT_HEIGHT = 80.0;  // cm — stop lifting above this
const float MIN_LIFT_HEIGHT = 10.0;  // cm — stop lowering below this

// WiFi credentials
const char* ssid = "Group_18_AP";
const char* password = "123";

unsigned long previousTime = 0;
const long interval = 500;

WiFiServer server(80);

String output26State = "CLOSE";
bool shipDetected = false;

// --- Motor control functions ---
void openBridge() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 50);  // PWM speed control (0–255)
}

void closeBridge() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, 50);
}

void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}

// --- Sensor reading ---
void sensors() {
  // Clear trigger pins
  digitalWrite(TRIG_PIN1, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN1, LOW);

  digitalWrite(TRIG_PIN2, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN2, LOW);

  long duration1 = pulseIn(ECHO_PIN1, HIGH);
  long duration2 = pulseIn(ECHO_PIN2, HIGH);

  distance1 = duration1 * 0.0343 / 2;
  distance2 = duration2 * 0.0343 / 2;

  Serial.print("Distance1/");
  Serial.print(count);
  Serial.print(": ");
  Serial.print(distance1);
  Serial.println(" cm");

  Serial.print("Distance2: ");
  Serial.print(distance2);
  Serial.println(" cm");

  count++;
}

// --- Web Page ---
String generateControls(int pin, const String& state) {
  String html = "<p>PIN " + String(pin) + " - Bridge Status " + state + "</p>";
  String nextAction = (state == "CLOSE") ? "OPEN" : "CLOSE";
  String buttonColor = (state == "CLOSE") ? "button" : "button button2";
  html += "<p><a href=\"/" + String(pin) + "/" + nextAction +
          "\"><button class=\"" + buttonColor + "\">" + nextAction + "</button></a></p>";

  String modeText = manualMode ? "MANUAL" : "AUTO";
  String nextMode = manualMode ? "AUTO" : "MANUAL";
  String modeColor = manualMode ? "button button2" : "button";
  html += "<p><a href=\"/MODE/" + nextMode +
          "\"><button class=\"" + modeColor + "\">" + nextMode + " MODE</button></a></p>";

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

  client.println(generateControls(output26, output26State));

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

void handleRequest(String& header, WiFiClient& client) {
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
  } else if (header.indexOf("GET /MODE/MANUAL") >= 0) {
    manualMode = true;
    manualOverride = true;
    Serial.println("Switched to MANUAL mode.");
  } else if (header.indexOf("GET /MODE/AUTO") >= 0) {
    manualMode = false;
    manualOverride = false;
    Serial.println("Switched to AUTO mode.");
  }

  sendWebPage(client);
}

// --- Setup ---
void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN1, OUTPUT);
  pinMode(ECHO_PIN1, INPUT);
  pinMode(TRIG_PIN2, OUTPUT);
  pinMode(ECHO_PIN2, INPUT);

  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  Serial.print("Setting Access Point...");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  stopMotor();
  server.begin();
}

// --- Main loop ---
void loop() {
  unsigned long currentTime = millis();
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

  // --- AUTO MODE CONTROL WITH STOP CONDITIONS ---
  if (!manualMode) {
    if ((distance1 > 0 && distance1 < 50) || (distance2 > 0 && distance2 < 50)) {
      Serial.println("Ship approaching, opening bridge...");
      shipDetected = true;
      digitalWrite(output26, HIGH);
      digitalWrite(output27, LOW);

      // --- NEW: Stop if bridge already fully open ---
      if (distance1 >= MAX_LIFT_HEIGHT || distance2 >= MAX_LIFT_HEIGHT) {
        Serial.println("Bridge fully open — stopping motor");
        stopMotor();
      } else {
        openBridge();
      }

    } else if (distance1 > 100 || distance2 > 100) {
      Serial.println("Ship has passed or no ship detected");
      shipDetected = false;
      digitalWrite(output26, LOW);
      digitalWrite(output27, HIGH);

      // --- NEW: Stop if bridge already fully closed ---
      if (distance1 <= MIN_LIFT_HEIGHT || distance2 <= MIN_LIFT_HEIGHT) {
        Serial.println("Bridge fully closed — stopping motor");
        stopMotor();
      } else {
        closeBridge();
      }

    } else {
      stopMotor();
    }
  }

  // --- Sensor check every 500ms ---
  if (currentTime - previousTime >= interval) {
    previousTime = currentTime;
    sensors();
  }
}
