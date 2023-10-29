#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

double percentage;
double voltage;

bool Display = false;

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

int offsetValue = 20; // set the correction offset value

unsigned long buttonPressedTime = 0;
unsigned long previousMillis = 0;  // variable to store the previous time
const long interval = 10000;       // interval at which to blink (milliseconds)


// Replace with your network credentials
const char* ssid = "FuturisticIOX IoT";
const char* password = "12345678";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
  lcd.print("Welcome!!");
  delay(3000);
  lcd.clear();
  lcd.noBacklight();

  pinMode(A0, INPUT); // analog input pin
  pinMode(D5, OUTPUT); // relay control pin
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);
  digitalWrite(D5, HIGH);
  digitalWrite(D6, HIGH);
  digitalWrite(D7, HIGH);
  digitalWrite(D8, HIGH);

  WiFi.softAP(ssid, password); // Set WiFi AP mode

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    int sensorValue = analogRead(A0); // read the analog input value
    voltage = map(sensorValue, 0, 1023, 0, 1565) + offsetValue; // map 0-1023 to 0-2500 and add correction offset
    voltage /= 100; // divide by 100 to get the decimal values
    percentage = ((voltage - 9.75) / (12.6 - 9.75)) * 100; // convert to percentage

    String html = "<html><head><style>";
    html += "body { font-size: 50px; text-align:center; }";
    html += "h1 { font-size: 36px; }";
    html += "p { font-size: 60px; }";
    html += "input[type='submit'] { font-size: 100px; padding: 40px; }";
    html += "</style></head><body>";
    html += "<h1>Battery Monitoring</h1>";
    html += "<p>Sensor Value: " + String(sensorValue) + "</p>";
    html += "<p>Battery Voltage: " + String(voltage) + "V</p>";
    html += "<p>Percentage: " + String(percentage) + "%</p>";
    html += "<h1>Control Buttons</h1>";
    html += "<form method='get' action='/relay'>";
    html += "<p><input type='submit' name='Display' value='Display'></p>";
    html += "<input type='submit' name='relay1' value='Relay 1'>";
    html += "<input type='submit' name='relay2' value='Relay 2'>";
    html += "<input type='submit' name='relay3' value='Relay 3'>";
    html += "<input type='submit' name='relay4' value='Relay 4'>";
    html += "</form>";
    html += "</body></html>";
    request->send(200, "text/html", html);



  });

  server.on("/relay", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasParam("Display")) {
      Display = true;
    }
    if (request->hasParam("relay1")) {
      digitalWrite(D5, !digitalRead(D5)); // toggle relay state
    }
    if (request->hasParam("relay2")) {
      digitalWrite(D6, !digitalRead(D6));
    }
    if (request->hasParam("relay3")) {
      digitalWrite(D7, !digitalRead(D7));
    }
    if (request->hasParam("relay4")) {
      digitalWrite(D8, !digitalRead(D8));
    }
    request->redirect("/");
  });

  server.begin();
}

void loop() {
  if (Display) {
    int sensorValue = analogRead(A0); // read the analog input value
    voltage = map(sensorValue, 0, 1023, 0, 1565) + offsetValue; // map 0-1023 to 0-2500 and add correction offset
    voltage /= 100; // divide by 100 to get the decimal values
    percentage = ((voltage - 9.75) / (12.6 - 9.75)) * 100; // convert to percentage

    lcd.begin();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Voltage: ");
    lcd.print(voltage);
    lcd.print("V");
    lcd.setCursor(0, 1);
    lcd.print("Percent: ");
    lcd.print(percentage);
    lcd.print("%");
    //      lcd.print("Volt: "+voltage+"V    ");
    //      lcd.print("percent: "+percentage+"%");
    previousMillis = millis();
    Display = false;

  }

  // check if it's time to turn off the LED
  unsigned long currentMillis = millis();
  if (Display == false && currentMillis - previousMillis >= interval) {
    lcd.clear();
    lcd.noBacklight();
  }

}
