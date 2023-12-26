//#include <TinyWireM.h>
#include <SoftwareSerial.h>

#include <SPI.h>
#include <Adafruit_BMP280.h>


SoftwareSerial mySerial(-1, 3);
Adafruit_BMP280 bmp;
 


void setup() {
  pinMode(1, OUTPUT);
  digitalWrite(1, HIGH);

  
  bmp.begin(0x76);
bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,
    Adafruit_BMP280::SAMPLING_X16,
    Adafruit_BMP280::FILTER_X16,
    Adafruit_BMP280::STANDBY_MS_500
  );


}

void loop() {
  mySerial.println();
  digitalWrite(1, HIGH);
  delay(1000);
  digitalWrite(1, LOW);
  delay(1000);

}
