#include <Servo.h>

Servo myServo;
int currentAngle = 40;
int minAngle = 20;
int maxAngle = 100;
int stepDelay = 15;  // Delay in milliseconds between steps (adjust for speed)

void setup() {
  Serial.begin(9600);
  myServo.attach(9);  
  myServo.write(currentAngle);
  Serial.println("Enter angle (20 to 100):");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    int angle = input.toInt();

    Serial.print("Received: ");
    Serial.println(input);
    Serial.print("Parsed angle: ");
    Serial.println(angle);

    // Fix your validation condition:
    if (angle >= minAngle && angle <= maxAngle) {
      moveServoSmoothly(currentAngle, angle);
      currentAngle = angle;
      Serial.print("Servo moved to: ");
      Serial.println(currentAngle);
    } else {
      Serial.println("Invalid angle. Enter 20 to 100.");
    }
  }
}

void moveServoSmoothly(int fromAngle, int toAngle) {
  if (fromAngle == toAngle) return;

  int step = (fromAngle < toAngle) ? 1 : -1;

  for (int pos = fromAngle; pos != toAngle; pos += step) {
    myServo.write(pos);
    delay(stepDelay);
  }
  myServo.write(toAngle); // Ensure exact position at the end
}
