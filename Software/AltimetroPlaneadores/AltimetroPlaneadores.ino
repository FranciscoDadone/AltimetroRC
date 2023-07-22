#include <Wire.h>
#include <Servo.h>
#include "SparkFunBME280.h"
#include <MsTimer2.h>

#define TIEMPO_MAX 100
#define ALTURA_MAX 2

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
  int tiempo = TIEMPO_MAX * 10;
  int sensor;
  int led;
  int led_d;
} tiempos;

int estado = ESPERA;

void setup() {
  pinMode(PIN_IN, INPUT_PULLUP);
  pinMode(13, OUTPUT);

  MsTimer2::set(100, interrupt);
  MsTimer2::start();

  Wire.begin();
  Serial.begin(9600);

  ESC.attach(PIN_OUT);

  bmp280.setI2CAddress(0x76);
  if(bmp280.beginI2C() == false) Serial.println("Fallo BMP280");
  
  ESC.writeMicroseconds(1000);
}

void interrupt() {
  if (tiempos.tiempo && estado == INICIADO) tiempos.tiempo--;
  if (tiempos.sensor) tiempos.sensor--;
  if (tiempos.led) tiempos.led--;
}

void leer_sensor (void);
void actualizar_led_altura (void);

void loop() {
  motor.vel = pulseIn(PIN_IN, HIGH);

  switch (estado) {
    case PARADO:
      ESC.writeMicroseconds(1000);
      leer_sensor();
      if (!tiempos.led) {
        if (tiempos.led_d) tiempos.led_d--;
        actualizar_led_altura(altura.maxima);
        tiempos.led = 5;
      }
      break;
    case ESPERA:
      if (motor.vel >= 1300) {
        leer_sensor();
        altura.inicial = altura.actual;
        estado = INICIADO;
        digitalWrite(13, HIGH);
      }
      break;
    case INICIADO:
      leer_sensor();
      ESC.writeMicroseconds(motor.vel);

      int diff = altura.actual - altura.inicial;
      
      if (!tiempos.tiempo || motor.vel < 1300 || (diff) >= ALTURA_MAX) {
        ESC.writeMicroseconds(1000);
        digitalWrite(13, LOW);
        estado = PARADO;
      }
      break;
  }
}


void leer_sensor () {
  if (tiempos.sensor) return;
  
  altura.actual = bmp280.readFloatAltitudeMeters();

  if (estado != ESPERA && altura.maxima < (altura.actual - altura.inicial)) altura.maxima = altura.actual - altura.inicial;
  
  tiempos.sensor = 1;
}


int reverse (int n) {
  int reverse = 0, remainder;
  while (n != 0) {
    remainder = n % 10;
    reverse = reverse * 10 + remainder;
    n /= 10;
  }
  return reverse;
}


int alt_viendo = 0;
int ult_digito;

void actualizar_led_altura (int metros) {
  if (tiempos.led_d) return;
  
  if (digitalRead(13)) {
    digitalWrite(13, LOW);
    tiempos.led_d = 1;
    return;
  }

  // Reinicia el número
  if (!alt_viendo && !ult_digito) {
    if (metros % 10 == 0) metros++;
    
    alt_viendo = reverse(metros);
    tiempos.led_d = 10;
    return;
  }

  // Lee los dígitos
  if (!ult_digito) {
    ult_digito = alt_viendo % 10;
    alt_viendo /= 10;
    tiempos.led_d = 4;
    if (!ult_digito) {
      tiempos.led_d = 0;
      delay(1000);
      digitalWrite(13, HIGH);
      delay(50);
      digitalWrite(13, LOW);
    }
  } else {
    tiempos.led_d = 1;
    ult_digito--;
    digitalWrite(13, HIGH);
  }
}
