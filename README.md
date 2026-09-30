ESP32 Ultrasonic Parking Sensor

A car-style parking sensor built on an ESP32, using an ultrasonic sensor to detect distance and a buzzer + LED to warn of nearby objects, with the alert rate increasing as the object gets closer.

Built as my individual project for ENG 2001 (Engineering Project Management), and as a personal hardware/embedded systems project.

Demo Video

[Watch the demo on YouTube](https://www.youtube.com/watch?v=CUzuEYRcckg
)

How it works

The HC-SR04 ultrasonic sensor sends out a sound pulse and times how long it takes to bounce back. The ESP32 converts that time into a distance in centimeters, then decides how to respond:

Distance	Buzzer	LED
> 25 cm	Silent	Off
5–25 cm	Beeping, faster as distance decreases	Flashing at the same rate
< 5 cm	Continuous tone	Solid on
Hardware
ESP32 Dev Module
HC-SR04 ultrasonic sensor
Active buzzer
LED
Breadboard + jumper wires
1kΩ and 2kΩ resistors (voltage divider, see below)
Why there's a voltage divider

The HC-SR04's Echo pin outputs a 5V signal, but the ESP32's GPIO pins are only rated for 3.3V. A simple two-resistor voltage divider (1kΩ / 2kΩ) steps the Echo signal down to a safe ~3.3V before it reaches the ESP32, protecting the pin from damage.

Reliability features

Raw ultrasonic sensor readings are noisy — a single reading can jump wildly due to echo interference, angled surfaces, or a missed echo. Three features address this:

Median filtering: 5 readings are taken per cycle, sorted, and the middle (median) value is used instead of a single raw reading. This eliminated the false triggers and boundary flickering seen during testing.
Impossible-reading rejection: readings of 0 or outside the sensor's physical range (0–400 cm) are discarded, and the last known valid reading is reused instead. This is separate from "far away" readings (e.g. 250 cm when nothing is nearby), which are valid and correctly treated as "nothing detected."
Disconnect detection: if 6 consecutive readings fail (roughly 1 second), the system assumes the sensor has been disconnected and shows a fault signal — a fast-flashing LED with the buzzer silenced — instead of behaving unpredictably.
Wiring
Component pin	Connects to
HC-SR04 Vcc	ESP32 5V
HC-SR04 Gnd	ESP32 GND
HC-SR04 Trig	ESP32 GPIO 5
HC-SR04 Echo	Voltage divider (1kΩ + 2kΩ) → ESP32 GPIO 18
Buzzer +	ESP32 GPIO 4
Buzzer −	ESP32 GND
LED +	ESP32 GPIO 2
LED −	ESP32 GND
What I'd improve next
Move the 5-reading sample loop to run on a timer/interrupt instead of blocking delay() calls, so the system could do other work between readings
Try averaging duration readings using micros()-based non-blocking timing instead of pulseIn(), for finer control
Add hysteresis at the 5cm and 25cm zone boundaries to further reduce any remaining flicker right at the edges
