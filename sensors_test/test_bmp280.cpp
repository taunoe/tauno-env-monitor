/* 
    https://github.com/adafruit/Adafruit_BMP280_Library
    SDA/SCL default to pins 4 & 5 
    but any two pins can be assigned as SDA/SCL using 
    Wire.begin(SDA,SCL)

    23. juuni 2020
    Tauno Erik
 */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

const int serial_speed {115200};
const int delay_time {2000};
// I2C
const int bmp280_addr {0x76}; // 

/* Init BMP280 */
Adafruit_BMP280 bmp;
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

/********************************
  Function to initialize BMP280
  use in setup()
*********************************/
void init_bmp280() {
  Serial.println(F("BMP280 Sensor event test"));

  if (!bmp.begin(bmp280_addr)) {
    Serial.println(F("Could not find a BMP280 sensor, check wiring or I2C!"));
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  bmp_temp->printSensorDetails();
}

/********************************
  Function to initialize BMP280
  use in setup()
*********************************/

/******************************************************/

void setup() {
  Serial.begin(serial_speed);
  
  init_bmp280();
}

void loop() {
  sensors_event_t temp_event, pressure_event;
  bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);
  
  Serial.print(F("Temperature = "));
  Serial.print(temp_event.temperature);
  Serial.println(" *C");

  Serial.print(F("Pressure = "));
  Serial.print(pressure_event.pressure);
  Serial.println(" hPa");

  Serial.println();
  delay(delay_time);
}

