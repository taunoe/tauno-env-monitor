/*****************************************************
 * File: main.cpp
 * By Tauno Erik
 * Modified: 14. juuli 2020
 *****************************************************/

#include <Arduino.h>
#include <SdsDustSensor.h>
#include <U8g2lib.h>        // Universal 8bit Graphics Library 
                            // (https://github.com/olikraus/u8g2/)
#include <Wire.h>           // I2C
#include <ccs811.h>

/* CCS811 co2 sensor 
******************************************************/
CCS811 ccs811(D3);            // nWAKE on D3 or GND
uint16_t ccs811_eco2 = 0; 
uint16_t ccs811_etvoc = 0;
uint16_t ccs811_errstat = 0; 
uint16_t ccs811_raw = 0;

/* SDS011 Dust Sensor 
******************************************************/
const int SDS_RX_PIN = D7; // D3 -> SDS011 TX pin
const int SDS_TX_PIN = D8; // D4 -> SDS011 RX pin
SoftwareSerial softwareSerial(SDS_RX_PIN, SDS_TX_PIN);
SdsDustSensor sds(softwareSerial); //  additional parameters: retryDelayMs and maxRetriesNotAvailable
float sds011_pm25 = 0;
float sds011_pm10 = 0;

/* Display */
U8X8_SH1106_128X64_NONAME_HW_I2C display (/* reset=*/ U8X8_PIN_NONE);

/* Timing */
const int ONE_MINUTE = 60000; 
const int FIVE_MINUTES = ONE_MINUTE * 5;
const int FIVE_MINUTES_30SEK = (ONE_MINUTE*5) + (ONE_MINUTE/2); // 5min 30sek

/* Timing booleans */
boolean is_1minute = false;
boolean is_5minutes = false;
boolean is_5minutes_30sek = false;

unsigned long prev_1minute_millis = 0;
unsigned long prev_5minutes_millis = 0;
unsigned long prev_5minutes_30sek = 0;

void setup() {
  Serial.begin(115200);

  /* Display 
  **************************************************/
  display.begin();
  display.setPowerSave(0);
  display.setFont(u8x8_font_amstrad_cpc_extended_r);

  // Print compile data
  display.setCursor(0, 0);         // Row 0, column 0
  display.print("Compiled:");

  display.setCursor(1, 1);         // Row 1, column 1
  display.print(__DATE__);         // Compile date

  display.setCursor(1, 2);         // Row 2, column 1
  display.print(__TIME__);         // Compile time

  display.setCursor(0, 3);         // Row 3, column 0
  display.print("by Tauno Erik");

  /* SDS011 Dust Sensor 
  ******************************************************/
  sds.begin();
  // Prints SDS011 firmware version:
  Serial.print("SDS011 ");
  Serial.println(sds.queryFirmwareVersion().toString()); 
  // Ensures SDS011 is in 'query' reporting mode:
  Serial.print("SDS011 ");
  Serial.println(sds.setQueryReportingMode().toString());

  /* CCS811 
  ******************************************************/
  Serial.print("CCS811 lib version: "); 
  Serial.println(CCS811_VERSION);
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok = ccs811.begin();
  if (!ok){
    Serial.println("CCS811 begin FAILED");
  } 
  // Print CCS811 versions
  Serial.print("CCS811 hardware    version: ");
  Serial.println(ccs811.hardware_version(),HEX);
  Serial.print("CCS811 bootloader  version: ");
  Serial.println(ccs811.bootloader_version(),HEX);
  Serial.print("CCS811 application version: ");
  Serial.println(ccs811.application_version(),HEX);
  // Start measuring
  ok = ccs811.start(CCS811_MODE_1SEC);
  if( !ok ){
    Serial.println("CCS811 start FAILED");
  } 

} // setup end

void loop() {

  /* Start measure millis 
  ********************************************************/
  unsigned long current_millis = millis();

  // 01:00
  if ((unsigned long)(current_millis - prev_1minute_millis) >= ONE_MINUTE){
    is_1minute = true;
    prev_1minute_millis = current_millis;
  }
  // 05:00
  if ((unsigned long)(current_millis - prev_5minutes_millis) >= FIVE_MINUTES){
    is_5minutes = true;
    prev_5minutes_millis = current_millis;
  }
  // 05:30
  if ((unsigned long)(current_millis - prev_5minutes_30sek) >= FIVE_MINUTES_30SEK){
    is_5minutes_30sek = true;
    prev_5minutes_30sek = current_millis;
  }

  /* Do things
  ********************************************************/
  if (is_1minute){
    // mõõda
    // CCS811
    ccs811.read(&ccs811_eco2,&ccs811_etvoc,&ccs811_errstat,&ccs811_raw);

    if (ccs811_errstat == CCS811_ERRSTAT_OK) {
      //Serial.print("raw6=");  Serial.print(ccs811_raw/1024); Serial.print(" uA  "); 
      //Serial.print("raw10="); Serial.print(ccs811_raw%1024); Serial.print(" ADC  ");
      //Serial.print("R="); Serial.print((1650*1000L/1023)*(ccs811_raw%1024)/(ccs811_raw/1024)); Serial.print(" ohm");
    } else if (ccs811_errstat == CCS811_ERRSTAT_OK_NODATA) {
      Serial.println("CCS811: waiting for (new) data");
    } else if (ccs811_errstat & CCS811_ERRSTAT_I2CFAIL) { 
      Serial.println("CCS811: I2C error");
    } else {
      Serial.print("CCS811: errstat="); Serial.print(ccs811_errstat,HEX); 
      Serial.print("="); Serial.println( ccs811.errstat_str(ccs811_errstat) ); 
    }

    // prindi
    Serial.print("PM2.5 = ");
    Serial.print(sds011_pm25); // float, μg/m3
    display.setCursor(0, 0);
    display.clearLine(0);
    display.print("PM2.5= ");
    display.setCursor(7, 0);
    display.print(sds011_pm25);
    Serial.print(", PM10 = ");
    Serial.println(sds011_pm10); // float, μg/m3
    display.setCursor(0, 1);
    display.clearLine(1);
    display.print("PM10 = ");
    display.setCursor(7, 1);
    display.print(sds011_pm10);

    display.setCursor(0, 2);
    display.clearLine(2);
    display.print("eco2 =");
    display.setCursor(7, 2);
    display.print(ccs811_eco2);

    display.setCursor(0, 3);
    display.clearLine(3);
    display.print("tvoc =");
    display.setCursor(7, 3);
    display.print(ccs811_etvoc);

    Serial.print("CCS811: ");
    Serial.print("eco2=");
    Serial.print(ccs811_eco2);
    Serial.print(" ppm  ");
    Serial.print("etvoc=");
    Serial.print(ccs811_etvoc);
    Serial.print(" ppb  ");
    Serial.println();
    //
    is_1minute = false;
  }

  // Do this every 5 minutes
  if (is_5minutes){
    sds.wakeup();
    is_5minutes = false;
  }

  // Do this every 5 minutes and 30 seconds
  if (is_5minutes_30sek){
    // Query SDS011
    PmResult pm = sds.queryPm();
    if (pm.isOk()) {
      sds011_pm25 = pm.pm25;
      sds011_pm10 = pm.pm10;
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

    is_5minutes_30sek = false;
  } // end is_sds_query

} // main loop end