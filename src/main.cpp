/*****************************************************
 * File: main.cpp
 * By Tauno Erik
 * Modified: 12. juuli 2020
 *****************************************************/

#include <Arduino.h>
#include <SdsDustSensor.h>
#include <U8g2lib.h>        // Universal 8bit Graphics Library 
                            // (https://github.com/olikraus/u8g2/)
#include <Wire.h>           // I2C


/* SDS011 Dust Sensor */
const int SDS_RX_PIN = D7; // D3 -> SDS011 TX pin
const int SDS_TX_PIN = D8; // D4 -> SDS011 RX pin
SoftwareSerial softwareSerial(SDS_RX_PIN, SDS_TX_PIN);
SdsDustSensor sds(softwareSerial); //  additional parameters: retryDelayMs and maxRetriesNotAvailable

/* Display */
U8X8_SH1106_128X64_NONAME_HW_I2C display (/* reset=*/ U8X8_PIN_NONE);

/* Timing */
const int MINUTE = 60000; 
const int SDS_WAKEUP_TIME = MINUTE*5;               // 5min
const int SDS_QUERY_TIME = (MINUTE*5) + (MINUTE/2); // 5min 30sek

/* Timing booleans */
boolean is_sds_wakeup = false;
boolean is_sds_query = false;

unsigned long prev_sds_wakeup_millis = 0;
unsigned long prev_sds_query_millis = 0;

void setup() {
  Serial.begin(115200);

  /* Display */
  display.begin();
  display.setPowerSave(0);
  display.setFont(u8x8_font_amstrad_cpc_extended_r);

  display.setCursor(0, 0);         // Row 0, column 0
  display.print("Compiled:");

  display.setCursor(1, 1);         // Row 1, column 1
  display.print(__DATE__);         // Compile date

  display.setCursor(1, 2);         // Row 2, column 1
  display.print(__TIME__);         // Compile time

  display.setCursor(0, 3);         // Row 3, column 0
  display.print("by Tauno Erik");

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
  // Start measure millis
  unsigned long current_millis = millis();

  if ((unsigned long)(current_millis - prev_sds_wakeup_millis) >= SDS_WAKEUP_TIME){
    is_sds_wakeup = true;
    prev_sds_wakeup_millis = current_millis;
  }
  if ((unsigned long)(current_millis - prev_sds_query_millis) >= SDS_QUERY_TIME){
    is_sds_query = true;
    prev_sds_query_millis = current_millis;
  }

  // Wake up SDS011
  if (is_sds_wakeup){
    sds.wakeup();
    is_sds_wakeup = false;
  }

  // Get data from SDS011
  if (is_sds_query){
    PmResult pm = sds.queryPm();
    if (pm.isOk()) {
      Serial.print("PM2.5 = ");
      Serial.print(pm.pm25); // float, μg/m3

      display.setCursor(0, 0);
      display.clearLine(0);
      display.print("PM2.5= ");
      display.setCursor(6, 0);
      display.print(pm.pm25);

      Serial.print(", PM10 = ");
      Serial.println(pm.pm10); // float, μg/m3

      display.setCursor(0, 1);
      display.clearLine(1);
      display.print("PM10 = ");
      display.setCursor(6, 1);
      display.print(pm.pm10);

      display.clearLine(2);
      display.clearLine(3);

    } else {
      Serial.print("Could not read values from SDS011 sensor, reason: ");
      Serial.println(pm.statusToString());
    }

    // Put SDS011 back to sleep
    WorkingStateResult sds_state = sds.sleep();
    if (sds_state.isWorking()) {
      Serial.println("Problem with sleeping the SDS011 sensor.");
    } else {
      Serial.println("SDS011 sensor is sleeping");
    }

    is_sds_query = false;
  } // end is_sds_query

} // main loop end