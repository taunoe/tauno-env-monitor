/*****************************************************
 * File: main.cpp
 * By Tauno Erik
 * Modified: 15. juuli 2020
 *****************************************************/

#include <Arduino.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_AM2320.h>

Adafruit_AM2320 am2320 = Adafruit_AM2320();
float am2320_temp {0};
float am2320_hum {0};

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // hang out until serial port opens
  }

  Serial.println("Adafruit AM2320 Basic Test");
  am2320.begin();
}

void loop() {
  am2320_temp = am2320.readTemperature();
  am2320_hum = am2320.readHumidity();

  Serial.print("Temp: "); Serial.println(am2320_temp);
  Serial.print("Hum: "); Serial.println(am2320_hum);

  delay(2000);
}