/*
  SMART ROOF
  Automatic Rain Protection System

  Controller : Arduino Uno / Nano
  Sensor     : Rain Sensor Module
  Motor      : DC Gear Motor
  Driver     : L298N / compatible motor driver
  Switches   : 2 Limit Switches

  Working:
  - Rain detected  -> Roof/Rack moves to SAFE position
  - Safe limit     -> Motor stops
  - Rain stops     -> After delay, Roof/Rack returns
  - Home limit     -> Motor stops
*/

// ---------------- PIN CONFIGURATION ----------------

// Rain Sensor
const int RAIN_SENSOR = A0;

// Motor Driver
const int MOTOR_IN1 = 8;
const int MOTOR_IN2 = 9;
const int MOTOR_EN  = 10;

// Limit Switches
const int HOME_LIMIT = 6;   // Outdoor/Home position
const int SAFE_LIMIT = 7;   // Protected/Safe position


// ---------------- SETTINGS ----------------

// Change this value after testing your rain sensor.
// Lower value = more wet for many analog rain modules.
const int RAIN_THRESHOLD = 500;

// Time to wait after rain stops before returning the roof
const unsigned long RETURN_DELAY = 30000UL; // 30 seconds


// ---------------- VARIABLES ----------------

enum RoofState {
  OUTSIDE,
  MOVING_INSIDE,
  INSIDE,
  MOVING_OUTSIDE
};

RoofState state = OUTSIDE;

unsigned long rainStoppedTime = 0;


// ---------------- SETUP ----------------

void setup() {

  Serial.begin(9600);

  pinMode(RAIN_SENSOR, INPUT);

  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_EN, OUTPUT);

  pinMode(HOME_LIMIT, INPUT_PULLUP);
  pinMode(SAFE_LIMIT, INPUT_PULLUP);

  stopMotor();

  Serial.println("================================");
  Serial.println("       SMART ROOF SYSTEM");
  Serial.println("================================");
  Serial.println("System Started");
}


// ---------------- MAIN LOOP ----------------

void loop() {

  int rainValue = analogRead(RAIN_SENSOR);

  bool raining = rainValue < RAIN_THRESHOLD;

  bool homeReached = digitalRead(HOME_LIMIT) == LOW;
  bool safeReached = digitalRead(SAFE_LIMIT) == LOW;

  Serial.print("Rain Sensor: ");
  Serial.print(rainValue);

  Serial.print(" | Rain: ");
  Serial.print(raining ? "YES" : "NO");

  Serial.print(" | State: ");
  Serial.println(state);


  // =================================================
  // OUTSIDE POSITION
  // =================================================

  if (state == OUTSIDE) {

    stopMotor();

    if (raining) {

      Serial.println("Rain detected!");
      Serial.println("Moving roof to SAFE position...");

      state = MOVING_INSIDE;
    }
  }


  // =================================================
  // MOVING INSIDE
  // =================================================

  if (state == MOVING_INSIDE) {

    if (safeReached) {

      stopMotor();

      Serial.println("Safe position reached.");

      state = INSIDE;
    }
    else {

      moveInside();
    }
  }


  // =================================================
  // INSIDE / SAFE POSITION
  // =================================================

  if (state == INSIDE) {

    stopMotor();

    if (!raining) {

      if (rainStoppedTime == 0) {

        rainStoppedTime = millis();

        Serial.println("Rain stopped.");
        Serial.println("Waiting before returning...");
      }

      if (millis() - rainStoppedTime >= RETURN_DELAY) {

        Serial.println("Returning roof to OUTSIDE position.");

        rainStoppedTime = 0;

        state = MOVING_OUTSIDE;
      }
    }
    else {

      rainStoppedTime = 0;
    }
  }


  // =================================================
  // MOVING OUTSIDE
  // =================================================

  if (state == MOVING_OUTSIDE) {

    if (homeReached) {

      stopMotor();

      Serial.println("Outdoor position reached.");

      state = OUTSIDE;
    }
    else {

      moveOutside();
    }
  }


  delay(100);
}


// =================================================
// MOTOR FUNCTIONS
// =================================================

void moveInside() {

  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);

  analogWrite(MOTOR_EN, 200);
}


void moveOutside() {

  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);

  analogWrite(MOTOR_EN, 200);
}


void stopMotor() {

  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);

  analogWrite(MOTOR_EN, 0);
}
