  /* Blind Stick with GPS, Ultrasonic Sensor & Bluetooth */
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// Pin Definitions
#define TRIG_PIN 9
#define ECHO_PIN 10
#define BUZZER_PIN 13
#define SWITCH_PIN 4  // Emergency switch

// GPS & Bluetooth Serial Communication
TinyGPSPlus gps;
SoftwareSerial gps_serial(3, 2);  // GPS Module (RX, TX)
SoftwareSerial btSerial(5, 6);    // Bluetooth HC-05/HC-06 (RX, TX)

// GPS Variables
double LATITUDE, LONGITUDE;
String latitude = "", longitude = "";
double SPEED;
String SMS;

long previousDistance = 100;
unsigned long lastCheckTime = 0;

void setup() {
    Serial.begin(9600);        // Serial Monitor
    gps_serial.begin(9600);    // GPS Module
    btSerial.begin(9600);      // Bluetooth Module

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(SWITCH_PIN, INPUT_PULLUP);

    digitalWrite(BUZZER_PIN, LOW);
    Serial.println(F("Blind Stick Initialized"));
    btSerial.println(F("Blind Stick Bluetooth Connected"));
}



// Convert double to string with correct precision
String double_string_con(double input) {
    return String(input, 6); // Keep 6 decimal places
}

// Function to get distance from ultrasonic sensor
long getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 20ms timeout
    if (duration == 0) return previousDistance;

    long distance = duration * 0.034 / 2;
    return distance;
}

void loop() {
    while (gps_serial.available() > 0) gps.encode(gps_serial.read());

    if (millis() > 120000 && gps.charsProcessed() < 100000) {
        Serial.println(F("No GPS detected: check wiring."));
        btSerial.println(F("No GPS detected: check wiring."));
    }

    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime >= 50) {
        lastCheckTime = currentTime;
        
        long currentDistance = getDistance();

        // Detect fast-moving objects
        if ((previousDistance - currentDistance) > 20 && currentDistance > 0 && currentDistance < 30) {
            digitalWrite(BUZZER_PIN, HIGH);
            Serial.print("Fast-moving object detected at: ");
            Serial.print(currentDistance);
            Serial.println(" cm");
            btSerial.print("Fast-moving object detected at: ");
            btSerial.print(currentDistance);
            btSerial.println(" cm");
            delay(300);
            digitalWrite(BUZZER_PIN, LOW);
        }

        previousDistance = currentDistance;
    }

    // If switch is pressed, send GPS location alert
    if (digitalRead(SWITCH_PIN) == LOW) {
        digitalWrite(BUZZER_PIN, HIGH);

        if (gps.location.isValid()) {
            LATITUDE = gps.location.lat();
            LONGITUDE = gps.location.lng();

            latitude = double_string_con(LATITUDE);
            longitude = double_string_con(LONGITUDE);
            SPEED = gps.speed.kmph();
        } else {
            Serial.println(F("INVALID GPS DATA"));
            btSerial.println(F("INVALID GPS DATA"));
        }

        // Generate alert message
        SMS = "Caution: Emergency Button Pressed!\n";
        SMS += "Latitude: " + latitude + "\n";
        SMS += "Longitude: " + longitude + "\n";
        SMS += "Speed: " + String(SPEED) + " km/h\n";
        SMS += "Google Maps: https://www.google.com/maps/search/?api=1&query=";
        SMS += latitude + "," + longitude;

        Serial.println(SMS);
        btSerial.println(SMS); // Send to Bluetooth App

        delay(2000);
        digitalWrite(BUZZER_PIN, LOW);
    }
}
