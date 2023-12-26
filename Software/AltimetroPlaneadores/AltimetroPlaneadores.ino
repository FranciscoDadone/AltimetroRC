#include <Wire.h>
#include <Servo.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <MsTimer2.h>

#define TIEMPO_MAX 30
#define ALTURA_MAX 100

#define PIN_IN 2
#define PIN_OUT 9

#define ESPERA 10
#define INICIADO 20
#define PARADO 30

Adafruit_BMP280 bmp;
Servo ESC;

struct {
  float inicial;
  float actual;
  float maxima;
} altura;

struct {
  int vel;
  boolean apagado;
  int min_vel;
} motor;

struct {
  int tiempo = TIEMPO_MAX * 10;
  int sensor;
  int led;
  int led_d;
  int aviso_corte;
} tiempos;

int estado = ESPERA;

boolean deshabilitar_corte;

void setup() {
  pinMode(PIN_IN, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(A6, INPUT_PULLUP);
  digitalWrite(11, HIGH);

  MsTimer2::set(100, interrupt);
  MsTimer2::start();

  Wire.begin();
  Serial.begin(9600);

  Serial.println("Iniciando Altimetro...");

  ESC.attach(PIN_OUT);

  Serial.println("Iniciando BMP");
  bmp.begin(0x76);

  bmp.setSampling(
    Adafruit_BMP280::MODE_FORCED,
    Adafruit_BMP280::SAMPLING_X2,
    Adafruit_BMP280::SAMPLING_X16,
    Adafruit_BMP280::FILTER_X16,
    Adafruit_BMP280::STANDBY_MS_500
  );

  Serial.println("BMP iniciado");
  
  deshabilitar_corte = analogRead(A6) == 0;
  Serial.print("Corte de motor habilitado: ");
  Serial.println(!deshabilitar_corte);

  ESC.writeMicroseconds(900);
 
}

void interrupt() {
  if (tiempos.tiempo && estado == INICIADO) tiempos.tiempo--;
  if (tiempos.sensor) tiempos.sensor--;
  if (tiempos.led) tiempos.led--;
  if (tiempos.aviso_corte) tiempos.aviso_corte--;
}

void leer_sensor (void);
void actualizar_led_altura (void);
void aviso_corte(void);

boolean cortado = false;

void loop() {
  motor.vel = pulseIn(PIN_IN, HIGH);

  if (motor.min_vel == 0 || (motor.vel != 0 && motor.vel < motor.min_vel)) motor.min_vel = motor.vel;

  switch (estado) {
    case PARADO:
      if (deshabilitar_corte) ESC.writeMicroseconds(motor.vel);
      else if (!cortado) {
        ESC.writeMicroseconds(900);
        cortado = true;
      }
      
      leer_sensor();
      if (!tiempos.led) {
        if (tiempos.led_d) tiempos.led_d--;
        actualizar_led_altura(altura.maxima);
        tiempos.led = 5;
      }
      break;
    case ESPERA:
      leer_sensor();
      altura.inicial = altura.actual;

      if (!deshabilitar_corte) aviso_corte();

      ESC.writeMicroseconds(motor.vel);
      
      if (motor.vel >= motor.min_vel + 200) {
        estado = INICIADO;
        digitalWrite(13, HIGH);
      }
      break;
    case INICIADO:
      leer_sensor();
      ESC.writeMicroseconds(motor.vel);

      int diff = altura.actual - altura.inicial;
      
      if (!tiempos.tiempo || motor.vel < (motor.min_vel + 30) || diff >= ALTURA_MAX) { // 1300
        if (!deshabilitar_corte) ESC.writeMicroseconds(900);
        digitalWrite(13, LOW);
        estado = PARADO;
      }
      break;
  }
}

void leer_sensor () {
  if (tiempos.sensor) return;
  
  altura.actual = bmp.readAltitude(1013.25);
  Serial.println(altura.actual);

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

void aviso_corte() {
  if (tiempos.aviso_corte) return;

  digitalWrite(13, !digitalRead(13));

  tiempos.aviso_corte = 1;
}
