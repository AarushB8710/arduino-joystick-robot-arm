#include <Servo.h>

// --- Pin Allocations ---
const int PIN_BASE     = 9;
const int PIN_SHOULDER = 10;
const int PIN_ELBOW    = 11;
const int PIN_GRIPPER  = 6;

const int PIN_JOY_X    = A0;
const int PIN_JOY_Y    = A1;
const int PIN_JOY_SW   = 2;  // Joystick push button

// --- Servo Instances ---
Servo baseServo;
Servo shoulderServo;
Servo elbowServo;
Servo gripperServo;

// --- Calibration Angles ---
// Customize these values based on your physical cardboard arm's safety limits
const int MIN_BASE = 0,   MAX_BASE = 180;
const int MIN_SHLD = 15,  MAX_SHLD = 165;
const int MIN_ELBW = 15,  MAX_ELBW = 165;
const int GRIP_OPEN = 40, GRIP_CLOSED = 130; 

// --- Initial Startup Angles ---
float currentBase     = 90.0;
float currentShoulder = 90.0;
float currentElbow    = 90.0;
bool isGripperOpen    = true;

// --- Control Settings ---
const int DEADZONE = 50;       // Ignores minor stick drift near center (512)
const float MAX_SPEED = 1.5;   // Keep speed low to minimize sudden current spikes
const unsigned long GRIP_HOLD_TIME = 600; // How long (ms) to hold button to trigger gripper

// --- State Variables ---
bool controlMode = 0;          // 0 = Shoulder Mode, 1 = Elbow Mode
bool buttonIsPressed = false;
unsigned long buttonPressTime = 0;
bool gripperToggledThisPress = false;
bool lastButtonReading = HIGH;

void setup() {
  Serial.begin(115200);

  // Attach servos
  baseServo.attach(PIN_BASE);
  shoulderServo.attach(PIN_SHOULDER);
  elbowServo.attach(PIN_ELBOW);
  gripperServo.attach(PIN_GRIPPER);

  // Initialize servos to start positions
  baseServo.write((int)currentBase);
  shoulderServo.write((int)currentShoulder);
  elbowServo.write((int)currentElbow);
  gripperServo.write(isGripperOpen ? GRIP_OPEN : GRIP_CLOSED);

  // Configure joystick switch with internal pull-up resistor
  pinMode(PIN_JOY_SW, INPUT_PULLUP);

  Serial.println("System Initialized.");
  Serial.println("-> QUICK TAP the button to switch between Shoulder & Elbow mode.");
  Serial.println("-> PRESS AND HOLD the button (0.6s) to Open/Close the Gripper.\n");
}

void loop() {
  // 1. Read Inputs
  int rawX = analogRead(PIN_JOY_X);
  int rawY = analogRead(PIN_JOY_Y);
  bool currentButtonReading = digitalRead(PIN_JOY_SW); // LOW when pressed, HIGH when idle

  // 2. Button State Machine (Tap for Mode, Hold for Gripper)
  unsigned long now = millis();

  if (currentButtonReading == LOW) {
    if (!buttonIsPressed) {
      // Button was just pressed down
      buttonIsPressed = true;
      buttonPressTime = now;
      gripperToggledThisPress = false;
    }
    
    // If the button is held down past the threshold, trigger the gripper immediately
    if (!gripperToggledThisPress && (now - buttonPressTime >= GRIP_HOLD_TIME)) {
      isGripperOpen = !isGripperOpen;
      gripperServo.write(isGripperOpen ? GRIP_OPEN : GRIP_CLOSED);
      
      Serial.println("\n>>> [EVENT] GRIPPER TOGGLED via Hold <<<");
      gripperToggledThisPress = true; // Prevent triggering multiple times in one hold
    }
  } 
  else {
    if (buttonIsPressed) {
      // Button was just released
      unsigned long pressDuration = now - buttonPressTime;
      buttonIsPressed = false;
      
      // If they released the button before the hold threshold, switch the joint mode
      if (pressDuration < GRIP_HOLD_TIME) {
        controlMode = !controlMode; // Toggle between 0 (Shoulder) and 1 (Elbow)
        
        Serial.println("\n>>> [EVENT] JOINT MODE SWITCHED <<<");
      }
    }
  }

  // 3. Process Joystick Speeds (Incremental Control)
  float baseStep = getIncrement(rawX);
  float verticalStep = getIncrement(rawY);

  // Apply base movements (always active on X-axis)
  currentBase = constrain(currentBase + baseStep, MIN_BASE, MAX_BASE);
  baseServo.write((int)currentBase);

  // Apply vertical movements to either Shoulder or Elbow based on controlMode
  if (controlMode == 1) {
    // Elbow Mode
    currentElbow = constrain(currentElbow + verticalStep, MIN_ELBW, MAX_ELBW);
    elbowServo.write((int)currentElbow);
  } else {
    // Shoulder Mode
    currentShoulder = constrain(currentShoulder - verticalStep, MIN_SHLD, MAX_SHLD); // Inverted to match natural up-is-raise behavior
    shoulderServo.write((int)currentShoulder);
  }

  // 4. Print Status (Diagnostic Output)
  static unsigned long lastPrint = 0;
  if (now - lastPrint > 250) { // Limit frequency to keep the Serial Monitor readable
    lastPrint = now;
    printDiagnosticData();
  }

  delay(20); // Small cycle delay to steady servo motion rates
}

// Map analog readings to incremental adjustments based on stick offset
float getIncrement(int rawValue) {
  int offset = rawValue - 512;
  
  if (abs(offset) < DEADZONE) {
    return 0.0; // Stay inside deadzone
  }

  // Map remaining range smoothly to a fractional degree step size
  if (offset > 0) {
    return map(rawValue, 512 + DEADZONE, 1023, 1, 100) / 100.0 * MAX_SPEED;
  } else {
    return map(rawValue, 0, 512 - DEADZONE, -100, -1) / 100.0 * MAX_SPEED;
  }
}

// Print formatted arm positioning values
void printDiagnosticData() {
  Serial.print("Base: "); Serial.print((int)currentBase); Serial.print("° | ");
  Serial.print("Shoulder: "); Serial.print((int)currentShoulder); Serial.print("° | ");
  Serial.print("Elbow: "); Serial.print((int)currentElbow); Serial.print("° | ");
  Serial.print("Gripper: "); Serial.print(isGripperOpen ? "OPEN " : "CLOSE"); 
  Serial.print(" | Mode: ");
  Serial.println(controlMode == 1 ? "[ELBOW CONTROL ACTIVE]" : "[SHOULDER CONTROL ACTIVE]");
}