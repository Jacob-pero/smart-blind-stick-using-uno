#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define SS_PIN 10   // RFID SS (SDA) pin
#define RST_PIN 9   // RFID Reset pin
#define DF_TX 5     // DFPlayer TX (DFPlayer → Arduino)
#define DF_RX 6     // DFPlayer RX (Arduino → DFPlayer)

MFRC522 rfid(SS_PIN, RST_PIN); // RFID instance
SoftwareSerial mp3Serial(DF_RX, DF_TX); // DFPlayer Mini
DFRobotDFPlayerMini mp3Player; // DFPlayer Instance

void setup() {
    Serial.begin(9600);       // Serial Monitor
    mp3Serial.begin(9600);    // DFPlayer Mini Serial
    SPI.begin();
    rfid.PCD_Init();

    Serial.println("RFID + DFPlayer Ready.");

    if (!mp3Player.begin(mp3Serial)) {  
        Serial.println("DFPlayer Mini not detected!");
        while (true); // Halt if DFPlayer Mini is not connected
    }

    mp3Player.volume(30); // Set max volume (0-30)
}

void loop() {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        Serial.print("RFID Tag Detected: ");

        String tagID = "";
        for (byte i = 0; i < rfid.uid.size; i++) {
            tagID += String(rfid.uid.uidByte[i], HEX);
        }

        Serial.println(tagID);

        // Play audio based on the RFID tag
        if (tagID == "E1EB8E02") {
            mp3Player.play(1);  // Play "0001.mp3"
        } else if (tagID == "e1eb8e2") {
            mp3Player.play(2);  // Play "0002.mp3"
        } else {
            mp3Player.play(3);  // Default sound
        }

        rfid.PICC_HaltA(); // Stop reading the same card repeatedly
    }
}
