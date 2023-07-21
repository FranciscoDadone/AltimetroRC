#include <Wire.h>
#include <Servo.h>
#include "SparkFunBME280.h"

#define TIEMPO_MAX 30
#define ALTURA_MAX 100

#define PIN_IN 3
#define PIN_OUT 2

BME280 bmp280;
Servo ESC;

float altura;
float altura_0;
boolean iniciado = false;
int vel_motor;
long tiempo_0;
boolean motor_apagado = false;

void setup() {
  pinMode(PIN_IN, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  
  Wire.begin();
  Serial.begin(9600);

  ESC.attach(PIN_OUT);

  bmp280.setI2CAddress(0x76);
  if(bmp280.beginI2C() == false) Serial.println("Fallo BMP280");
  
  ESC.writeMicroseconds(1000);
  delay(5000);
}

void loop() {
  vel_motor = pulseIn(PIN_IN, HIGH);
  bmp280.readTempC();
  altura = bmp280.readFloatAltitudeMeters();
  
  if (!iniciado && vel_motor >= 1300) {
    iniciado = true;
    tiempo_0 = millis();
    altura_0 = bmp280.readFloatAltitudeMeters();
    digitalWrite(13, HIGH);
  }

  if (iniciado && vel_motor < 1300) {
    ESC.writeMicroseconds(1000);
    digitalWrite(13, LOW);
    motor_apagado = true;
  }

  if (iniciado && !motor_apagado) {
    if ((millis() - tiempo_0) / 1000 < TIEMPO_MAX && altura - altura_0 <= ALTURA_MAX) {
      ESC.writeMicroseconds(vel_motor);
    } else {
      ESC.writeMicroseconds(1000);
      digitalWrite(13, LOW);
      motor_apagado = true;
    }
  }
}
