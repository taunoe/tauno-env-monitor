/*****************************************************
 * File: main.cpp
 * By Tauno Erik
 * Modified: 12. juuli 2020
 *****************************************************/

#include <Arduino.h>
#include <SdsDustSensor.h>
#include <U8g2lib.h> // Universal 8bit Graphics Library 
                     // (https://github.com/olikraus/u8g2/)
#include <Wire.h>

const char compile_date[] = __DATE__ " " __TIME__;

/* SDS011 Dust Sensor */
const int SDS_RX_PIN = D7; // D3 -> SDS011 TX pin
const int SDS_TX_PIN = D8; // D4 -> SDS011 RX pin
SoftwareSerial softwareSerial(SDS_RX_PIN, SDS_TX_PIN);
SdsDustSensor sds(softwareSerial); //  additional parameters: retryDelayMs and maxRetriesNotAvailable

/* Display */
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);

const int MINUTE = 60000;
const int WAKEUP_WORKING_TIME = MINUTE / 2; // 30 seconds.
const int MEASUREMENT_INTERVAL = MINUTE * 5;

void setup() {
  Serial.begin(115200);

  /* Display */
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_r);

  u8x8.setCursor(0, 0);
  u8x8.print("Compiled:");

  u8x8.setCursor(1, 1);
  u8x8.print(__DATE__);

  u8x8.setCursor(1, 2);
  u8x8.print(__TIME__);

  u8x8.setCursor(0, 3);
  u8x8.print("by Tauno Erik");

  /* SDS011 Dust Sensor */
  sds.begin();
  // Prints SDS011 firmware version:
  Serial.print("SDS011 ");
  Serial.println(sds.queryFirmwareVersion().toString()); 
  // Ensures SDS011 is in 'query' reporting mode:
  Serial.print("SDS011 ");
  Serial.println(sds.setQueryReportingMode().toString());

} // setup end

void loop() {

  // Wake up SDS011
  sds.wakeup();
  delay(WAKEUP_WORKING_TIME);

  // Get data from SDS011
  PmResult pm = sds.queryPm();
  if (pm.isOk()) {
    Serial.print("PM2.5 = ");
    Serial.print(pm.pm25); // float, μg/m3
    u8x8.setCursor(0, 0);
    u8x8.clearLine(0);
    u8x8.print("PM2.5= ");
    u8x8.setCursor(6, 0);
    u8x8.print(pm.pm25);

    Serial.print(", PM10 = ");
    Serial.println(pm.pm10); // float, μg/m3
    u8x8.setCursor(0, 1);
    u8x8.clearLine(1);
    u8x8.print("PM10 = ");
    u8x8.setCursor(6, 1);
    u8x8.print(pm.pm10);

    u8x8.clearLine(2);
    u8x8.clearLine(3);

  } else {
    Serial.print("Could not read values from SDS011 sensor, reason: ");
    Serial.println(pm.statusToString());
  }

  // Put SDS011 back to sleep
  WorkingStateResult state = sds.sleep();
  if (state.isWorking()) {
    Serial.println("Problem with sleeping the SDS011 sensor.");
  } else {
    Serial.println("SDS011 sensor is sleeping");
    delay(MEASUREMENT_INTERVAL);
  }

} // main loop end