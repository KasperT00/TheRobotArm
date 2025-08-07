#include <SoftwareSerial.h>

// RX = Arduino pin 10 reads module TXD
// TX = Arduino pin 11 sends → module RXD
SoftwareSerial BTSerial(10, 11);

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }              // wait for USB Serial to be ready
  Serial.println("Starting Bluetooth test…");

  BTSerial.begin(9600);              // default VMA302 baud rate
  Serial.println("BTSerial ready at 9600");
}

void loop() {
  // 1) Bluetooth → USB Serial
  if (BTSerial.available()) {
    char c = BTSerial.read();
    Serial.write(c);
  }

  // 2) USB Serial → Bluetooth (optional)
  if (Serial.available()) {
    char c = Serial.read();
    BTSerial.write(c);
  }
}
