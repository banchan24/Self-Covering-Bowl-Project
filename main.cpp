#include <Servo.h>

// --- Pin map ----------------------------------------------------
// tiltPin   : digital input from tilt sensor (HIGH = stable, LOW = tipped?)
// ledPin    : status LED (OFF/ON by state)
// togglePin : on/off switch to arm/disarm the system
//-----------------------------------------------------------------
const int tiltPin   = 4;
const int ledPin    = 3;
const int togglePin = 5;

// Two servos controlling the cover/awning mechanism
// (Label colors kept from your original notes)
Servo servG; // green wire on D9
Servo servP; // purple wire on D10

void setup() {
  Serial.begin(9600);

  // I/O direction. If your switch/tilt uses internal pullups,
  // you can change to INPUT_PULLUP and invert logic accordingly.
  pinMode(ledPin, OUTPUT);
  pinMode(tiltPin, INPUT);
  pinMode(togglePin, INPUT);

  // Attach servos to their PWM pins
  servP.attach(10);
  servG.attach(9);

  // Put hardware in a safe default position
  digitalWrite(ledPin, LOW);
  servP.write(0);
  servG.write(50);

  Serial.println("Unspillabowl booted.");
}

void loop() {
  int state = 0; // 0 = disarmed/idle, 1 = armed, 2 = spill response
  int tilt   = digitalRead(tiltPin);
  int toggle = digitalRead(togglePin);

  // ---------------- State 0: Disarmed / Idle --------------------
  while (state == 0) {
    Serial.println("State 0 (idle)");
    digitalWrite(ledPin, LOW); // LED off when disarmed
    servP.write(0);            // park servos in open position
    servG.write(50);

    // Read toggle to arm the system
    toggle = digitalRead(togglePin);
    Serial.println(toggle == HIGH ? "toggle: HIGH (request arm)" : "toggle: LOW (stay idle)");

    if (toggle == HIGH) {
      state = 1; // move to armed state
    }
  }

  // ---------------- State 1: Armed / Monitoring -----------------
  while (state == 1) {
    Serial.println("State 1 (armed)");
    digitalWrite(ledPin, HIGH); // LED on when armed (ready)

    // Allow user to disarm at any time
    toggle = digitalRead(togglePin);
    Serial.println(toggle == HIGH ? "toggle: HIGH (remain armed)" : "toggle: LOW (disarm)");

    if (toggle == LOW) {
      state = 0;     // back to idle
      return;        // exit loop() so Arduino can re-enter fresh
    }

    // Watch for a tilt event that indicates a spill risk
    tilt = digitalRead(tiltPin);
    if (tilt == LOW) {  // assume LOW = tipped (depends on your sensor wiring)
      state = 2;        // go handle the spill
    }
  }

  // ---------------- State 2: Spill Response ---------------------
  while (state == 2) {
    Serial.println("State 2 (spill response)");
    digitalWrite(ledPin, LOW); // optional: LED off during motion
    servP.write(50);           // move cover into place
    servG.write(0);

    // Allow immediate disarm if user flips the switch
    toggle = digitalRead(togglePin);
    if (toggle == LOW) {
      state = 0; // back to idle; loop() restarts
    }

    // (Optional) You could add a timed hold or
    // wait-until-upright check here before returning to State 1.
  }
}
