#include <Wire.h>
#include <Servo.h>
#include "SparkFunBME280.h"

#define TIEMPO_MAX 30
#define ALTURA_MAX 100

#define PIN_IN 3
#define PIN_OUT 2

BME280 bmp280;
Servo ESC;

float altitude;
float initial_altitude;
boolean started;
int motor_speed;
long time_started;
boolean stop_motor;

void setup() {
  stop_motor = false;

  pinMode(PIN_IN, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  
  Wire.begin();
  Serial.begin(9600);

  ESC.attach(PIN_OUT);

  bmp280.setI2CAddress(0x76);
  if(bmp280.beginI2C() == false) Serial.println("Sensor A connect failed");

  started = false;
  
  ESC.writeMicroseconds(1000);
  delay(5000);
}

void loop() {
  motor_speed = pulseIn(PIN_IN, HIGH);
  bmp280.readTempC();
  altitude = bmp280.readFloatAltitudeMeters();
  
  if (!started && motor_speed >= 1300) {
    started = true;
    time_started = millis();
    initial_altitude = bmp280.readFloatAltitudeMeters();
    digitalWrite(13, HIGH);
  }

  if (started && motor_speed < 1300) {
    ESC.writeMicroseconds(1000);
    digitalWrite(13, LOW);
    stop_motor = true;
  }

  if (started && !stop_motor) {
    if ((millis() - time_started) / 1000 < TIEMPO_MAX && altitude - initial_altitude <= ALTURA_MAX) {
      ESC.writeMicroseconds(motor_speed);
    } else {
      ESC.writeMicroseconds(1000);
      digitalWrite(13, LOW);
      stop_motor = true;
    }
  }
}
