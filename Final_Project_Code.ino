/*
  Ultrasonic Parking Sensor
  ESP32 + HC-SR04 + Buzzer + LED

  Detects the distance to an object using an ultrasonic sensor and warns
  the driver with a buzzer and LED. The beep/flash rate increases as the
  object gets closer, similar to a real car's parking-assist system.

  Reliability features:
  - Median filtering across 5 readings to reduce sensor noise
  - Rejects impossible readings (0 or out of physical sensor range) and
    falls back to the last known valid reading
  - Detects a disconnected sensor within ~1 second and shows a fault
    signal (fast-flashing LED, buzzer silent)

  Author: Muzammil Gaho
*/

// ---- Pin definitions ----
const int TRIG_PIN = 5;
const int ECHO_PIN = 18;
const int BUZZER_PIN = 4;
const int LED_PIN = 2;

// ---- Distance thresholds (cm), per project spec ----
const int FAR_THRESHOLD = 25;    // beyond this: silent, LED off
const int CLOSE_THRESHOLD = 5;   // below this: continuous tone, LED on
const int MIN_VALID_DISTANCE = 0;
const int MAX_VALID_DISTANCE = 400; // HC-SR04's practical max range

// ---- Fault detection ----
const int FAULT_THRESHOLD = 6;   // consecutive bad readings before fault (~1 second)

// ---- State that must persist between loop() calls ----
float lastValidDistance = 0;
int failCount = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // ---- Step 1: take 5 raw distance readings ----
  // A short delay between pulses lets each echo fully clear before the
  // next pulse fires, avoiding interference between consecutive readings.
  float readings[5];

  for (int i = 0; i < 5; i++) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Timeout (30ms) prevents pulseIn() from hanging forever if the
    // sensor is disconnected or never sends an echo back.
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);
    readings[i] = duration * 0.0343 / 2;

    delay(30);
  }

  // ---- Step 2: sort the readings (simple bubble sort) ----
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4 - i; j++) {
      if (readings[j] > readings[j + 1]) {
        float temp = readings[j];
        readings[j] = readings[j + 1];
        readings[j + 1] = temp;
      }
    }
  }

  // ---- Step 3: take the median (middle value) as the filtered distance ----
  float distance = readings[2];

  // ---- Step 4: reject impossible readings, track consecutive failures ----
  if (distance > MIN_VALID_DISTANCE && distance <= MAX_VALID_DISTANCE) {
    lastValidDistance = distance;
    failCount = 0;
  } else {
    distance = lastValidDistance;
    failCount++;
  }

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // ---- Step 5: decide buzzer/LED behavior based on state ----
  if (failCount >= FAULT_THRESHOLD) {
    // FAULT: sensor appears disconnected. Buzzer off, LED fast-flashes.
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);

  } else if (distance > FAR_THRESHOLD) {
    // Far zone: nothing nearby, stay silent.
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);

  } else if (distance >= CLOSE_THRESHOLD && distance <= FAR_THRESHOLD) {
    // Middle zone: beep and flash, faster as the object gets closer.
    int beepDelay = map(distance, CLOSE_THRESHOLD, FAR_THRESHOLD, 100, 500);
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    delay(beepDelay);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    delay(beepDelay);

  } else if (distance < CLOSE_THRESHOLD) {
    // Close zone: continuous tone (rapid toggle) and solid LED.
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(10);
    digitalWrite(BUZZER_PIN, LOW);
    delay(10);
  }
}