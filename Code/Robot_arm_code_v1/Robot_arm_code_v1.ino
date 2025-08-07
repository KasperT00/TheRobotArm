#include <AccelStepper.h>
#include <Servo.h>

// Robot setup
// Five TB660 Drivers, two nema 23 motors and three nema 17 motors and a servo gripper

// Features
// Set to "Homing mode" where motors move relatively slow and user moves motors by hand to home positions
// - For all joints the homing direction is clockwise
// - No homing for gripper, as the servo knows its position

// Board used: STM32F407VE
// Define motor interfaces (AccelStepper: DRIVER mode = 1)
// In order from top to bottom
AccelStepper motor1(1, PA0, PA1); // Gripper nema 17
AccelStepper motor2(1, PA3, PA4); // Wrist nema 17 
AccelStepper motor3(1, PA6, PA7); // Middle Nema 17
AccelStepper motor4(1, PB10, PB11); // Shoulder Nema 23 
AccelStepper motor5(1, PB0, PB1); // Bottom Nema 23

// Define motor indexes
const int motorIndex1 = 0; // Gripper nema 17
const int motorIndex2 = 1; // Wrist nema 17
const int motorIndex3 = 2; // Middle nema 17
const int motorIndex4 = 3; // Shoulder nema 23
const int motorIndex5 = 4; // Bottom nema 23

// ENA pins (optional), Same as order above, top to bottom motor
const int enaPins[5] = {PA2, PA5, PB9, PB12, PB5};

// Five joystick components and their pins: VRX, VRY, SW
// Use only VRX for first four and both for fifth to control gripper too.
// Jostic Pins Analog pins only: VRX and VRY
// TODO Joysticks not in use yet. 
//const int pinVRX[5] = {PB12, PB13, PB14, PB15, PA8}; // VRX pins
//const int pinVRY[5] = {PB0, PB1, PB_2, PB3, PA_9};    // VRY pins Have _ in couple

// Define Servo Gripper pins MG996R
// Servo pins (optional, if using a servo gripper)
Servo gripperServo;
const int servoPin = PB8; // Servo pin?
unsigned long lastGripperMoveTime = 0;
int currentGripperAngle = 40;
const int gripperMinAngle = 20;  // As per your test
const int gripperMaxAngle = 100;
const int gripperSmoothDelay = 14; // ms delay per step for smooth movement
const boolean useServo = true;
int gripperJogDir = 0;  // -1 = closing, 1 = opening, 0 = idle


// Define bluetooth VMA302 component:
#define BTSerial Serial1  // USART1: PA9 (TX), PA10 (RX)

// Array of motor pointers for easier access
AccelStepper* motors[] = {&motor1, &motor2, &motor3, &motor4, &motor5};

// Motor specific parameters. Ratio, lengths between joints
const int baseToMiddle = 230;     // Distance from base to middle joint (mm)
const int middleToEnd = 200;      // Distance from middle to end joint (mm)
const int endToGripper = 50;      // Distance from end joint to gripper (mm)
const int endToGripperOffset = 0; // Offset for gripper position (mm)
const int horizontalGripperOffset = 0; // Offset in y-axis for gripper (mm)

long homePositions[5];   // Stores step position for each motor at home
const int gearboxRatio = 26;
const int stepsPerRevolution = 200; // for NEMA motors, typically 200 full steps/rev
const int offsetDegrees = 30;
const int maxDegreesFromHome = 180 - offsetDegrees; // 140°
float stepsPerDegree = (stepsPerRevolution * 8 * gearboxRatio) / 360.0;

const int maximumRotationForGripper = 60;    // Maximum rotation for gripper (degrees) from homing position

const int jogSpeed = 1000; // Steps per second
const int jogNormalSpeed = 900; // Steps per second
const int jogNormalSpeeds[5] = {1400, 1400, 900, 700, 800};  // Example values per motor
const int accel = 800;    // Steps/sec²
const int jogDelay = 20;  // Time to coast before stop (ms)

// Runtime control state
int motorJogDirs[5] = {0, 0, 0, 0, 0}; // Direction control for each motor
bool movementEnabled = true;

void setup() {
  Serial.begin(115200);

  // Set up ENA pins
  for (int i = 0; i < 5; i++) {
    pinMode(enaPins[i], OUTPUT);
    digitalWrite(enaPins[i], LOW); // Enable driver (LOW = enabled for TB6600)
    motors[i]->setMaxSpeed(jogNormalSpeeds[i]);
    motors[i]->setAcceleration(accel);
  }

  // Set up joystick pins
  /*for (int i = 0; i < 5; i++) {
    pinMode(pinVRX[i], INPUT);
    pinMode(pinVRY[i], INPUT);
  }*/

  // Set up servo pin
  if (useServo) {
    gripperServo.attach(servoPin);
    gripperServo.write(currentGripperAngle);  // Set to initial angle
    Serial.println("Servo initialized");
  }

  // Initialize bluetooth
  BTSerial.begin(9600);  // Start Bluetooth on Serial1 (PA9/PA10)
  BTSerial.println("Bluetooth Ready! ");
  Serial.println("Setup complete.");
}

void loop() {
  // Commands: 
  // 
  // Right
  // B0S0E0W0T0G:
  // B (bottom left) A (bottom right), C (shoulder up) D (shoulder down)
  // E (Elbow up) F(elbow down), G (wrist clockwise) H (wrist counterclockwise)
  // I (Wrist gripper spin clockwise) J(Wrist gripper spin counter clockwise),
  // K (gripper opening) L (Gripper closing)
  // S (Stop everything until W is received), W (allow movement)
  // P (Home position, allow arms to move maximum of 100 degrees from this point)
  // Also keep track of its position in 3d space when moving
  // Receive some command from bt serial of 3d space coordinates and move there in limits of home position minimum and maximum. TODO do not implement yet
  // Check if homing is needed, ie. do we have valid values for the "homes" if not, then this is required first. By starting homing by receiving "Z" and then user moves and sends "P"
  // Move the arm when command is given and stop when 0 is received. 
  // Multi moving also supported such as BC0 -> spins bottom and moves shoulder at the same time until the 0 is received OR the "S"

  // Some possible methods maybe
  checkBluetoothInput();
  handleMotorJog();
  handleGripperJog();
  runAllMotors();
}

// Check bluetooth input and move arm until S or 0 is received
void checkBluetoothInput() {
  while (BTSerial.available()) {
    char cmd = BTSerial.read();
    BTSerial.print("Received: ");
    BTSerial.println(cmd);

    switch (cmd) {
      case 'A': motorJogDirs[motorIndex5] = 1; break; // Bottom right shoul
      case 'B': motorJogDirs[motorIndex5] = -1; break; // Bottom left
      case 'C': motorJogDirs[motorIndex4] = 1; break; // Shoulder up bottom
      case 'D': motorJogDirs[motorIndex4] = -1; break; // Shoulder down
      case 'E': motorJogDirs[motorIndex3] = 1; break; // Elbow up
      case 'F': motorJogDirs[motorIndex3] = -1; break; // Elbow down
      case 'G': motorJogDirs[motorIndex2] = 1; break; // Wrist CW
      case 'H': motorJogDirs[motorIndex2] = -1; break; // Wrist CCW
      case 'I': motorJogDirs[motorIndex1] = 1; break; // Gripper rotation CW
      case 'J': motorJogDirs[motorIndex1] = -1; break; // Gripper rotation CCW
      case 'K': gripperJogDir = 1; break;   // Open gripper
      case 'L': gripperJogDir = -1; break;  // Close gripper
      case '0': 
        for (int i = 0; i < 5; i++) motorJogDirs[i] = 0;
        gripperJogDir = 0;
        break;
      case 'S': 
        movementEnabled = false;
        gripperJogDir = 0;
        break;
      case 'Q': // Save current positions as home
        for (int i = 0; i < 5; i++) {
          BTSerial.println(String(motors[i]->currentPosition()) + " | Joint " + String(i + 1) + ": " + String((motors[i]->currentPosition() - homePositions[i]) / stepsPerDegree) + "°");
          homePositions[i] = motors[i]->currentPosition();
        }
        Serial.println("Home positions saved.");
        break;
      case 'W': movementEnabled = true; break;
      default:
        BTSerial.print("Unknown command: ");
        BTSerial.println(cmd);
        break;
    }
  }
}

void handleGripperJog() {
  if (!useServo || !movementEnabled || gripperJogDir == 0) return;

  unsigned long now = millis();
  if (now - lastGripperMoveTime < gripperSmoothDelay) return;

  int newAngle = currentGripperAngle + gripperJogDir;

  if (newAngle >= gripperMinAngle && newAngle <= gripperMaxAngle) {
    currentGripperAngle = newAngle;
    gripperServo.write(currentGripperAngle);
    BTSerial.print("Gripper angle: ");  
    BTSerial.println(currentGripperAngle);
  } else {
    // Reached limit
    gripperJogDir = 0;
  }

  lastGripperMoveTime = now;
}

// Handle actual motor movements
void handleMotorJog() {
  for (int i = 0; i < 5; i++) {
    if (!movementEnabled || motorJogDirs[i] == 0) {
      motors[i]->stop();
      continue;
    }

    long direction = motorJogDirs[i];
    long target = motors[i]->currentPosition() + direction * jogNormalSpeeds[i];

    // Enforce limits based on home
    long deltaFromHome = target - homePositions[i];
    float deltaDegrees = deltaFromHome / stepsPerDegree;

    if (deltaDegrees > maxDegreesFromHome || deltaDegrees < -maxDegreesFromHome) {
      motors[i]->stop(); // Out of bounds
      BTSerial.println(String("Motor ") + i + " current pos: " + motors[i]->currentPosition() + " target: " + target + " delta: "+deltaDegrees+" maxdeg: "+maxDegreesFromHome);
      continue;
    }

    motors[i]->moveTo(target); // Valid move
  }
}

// Run all motors
void runAllMotors() {
  for (int i = 0; i < 5; i++) {
    motors[i]->run();
  }
}
