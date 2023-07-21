#include <Wire.h>
#include <Servo.h>
#include "SparkFunBME280.h"
#include <TimerOne.h>

#define TIEMPO_MAX 30
#define ALTURA_MAX 100

#define PIN_IN 3
#define PIN_OUT 2

#define ESPERA 10
#define INICIADO 20
#define PARADO 30

BME280 bmp280;
Servo ESC;

struct {
  float inicial;
  float actual;
  float maxima;
} altura;

struct {
  int vel;
  boolean apagado;
} motor;

struct {
  int tiempo;
  int sensor;
} tiempos;

int estado = ESPERA;

void setup() {
  pinMode(PIN_IN, INPUT_PULLUP);
  pinMode(13, OUTPUT);

  Timer1.initialize(1000);
  Timer1.attachInterrupt(interrupt_1ms);
  Wire.begin();
  Serial.begin(9600);

  ESC.attach(PIN_OUT);

  bmp280.setI2CAddress(0x76);
  if(bmp280.beginI2C() == false) Serial.println("Fallo BMP280");
  
  ESC.writeMicroseconds(1000);
  delay(5000);
}

void interrupt_1ms (void) {
  if (tiempos.tiempo && estado == INICIADO) tiempos.tiempo--;
  if (tiempos.sensor) tiempos.sensor--;
}

void leer_sensor (void);

void loop() {
  motor.vel = pulseIn(PIN_IN, HIGH);

  switch (estado) {
    case ESPERA:
      if (motor.vel >= 1300) {
        tiempos.tiempo = TIEMPO_MAX * 1000;
        leer_sensor();
        altura.inicial = altura.actual;
        estado = INICIADO;
        digitalWrite(13, HIGH);
      }
      break;
    case INICIADO:
      leer_sensor();
      ESC.writeMicroseconds(motor.vel);
      
      if (motor.vel < 1300 || !tiempos.tiempo || (altura.actual - altura.inicial) >= ALTURA_MAX) {
        ESC.writeMicroseconds(1000);
        digitalWrite(13, LOW);
        estado = PARADO;
      }
      break;
    case PARADO:
      ESC.writeMicroseconds(1000);
      leer_sensor();
      break;
  }
}


void leer_sensor () {
  if (tiempos.sensor) return;
  
  altura.actual = bmp280.readFloatAltitudeMeters();

  if (altura.maxima < altura.actual) altura.maxima = altura.actual;
  
  tiempos.sensor = 300;
}
