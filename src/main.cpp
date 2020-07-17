/*****************************************************
 * File: main.cpp
 * By Tauno Erik
 * Modified: 15. juuli 2020
 *****************************************************/

#include <Arduino.h>
#include <Wire.h>             // I2C
#include <SPI.h>
#include <SdsDustSensor.h>    // Dust sensor
#include <U8g2lib.h>          // Universal 8bit Graphics Library 
                              // (https://github.com/olikraus/u8g2/)
#include <ccs811.h>           // co2 sensor
#include <Adafruit_BMP280.h>  // Barometric Pressure
#include "APDS9960.h"         // Light, RGB, Proximity, Gesture sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_AM2320.h>
#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include <ThingSpeak.h> 

const int LED_1 {D6};
const int LED_2 {D5};
const int LED_3 {D0};
const int LED_4 {D4}; // allso on board LED!

void setup_leds() {
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, LOW);
  digitalWrite(LED_4, HIGH); // HIGH = on board led off
}

/******************************************************
 * 0.
 * Wifi & Thingspeak
 ******************************************************/
#include "secrets.h" 
// File secrets.h content:
// #define SECRET_SSID "MySSID"     // replace MySSID with your WiFi network name
// #define SECRET_PASS "MyPassword" // replace MyPassword with your WiFi password
const char * ssid = SECRET_SSID_1;   // your network SSID (name) 
const char * pass = SECRET_PASS_1;   // your network password
unsigned long myChannelNumber = THINGSPEAK_CH_ID;
const char * myWriteAPIKey = THINGSPEAK_WRITE_APIKEY;
WiFiClient  client;

String myStatus = "Test";

void status_message() {
// figure out the status message
  /*
  if(number1 > number2){
    myStatus = String("field1 is greater than field2"); 
  }
  else if(number1 < number2){
    myStatus = String("field1 is less than field2");
  }
  else{
    myStatus = String("field1 equals field2");
  }
  */
  myStatus = String("Ok");
}


/******************************************************
 * 1. 
 * CCS811 co2 sensor 
 ******************************************************/
CCS811 ccs811(D3);            // nWAKE on D3 or GND
uint16_t ccs811_eco2 = 0; 
uint16_t ccs811_etvoc = 0;
uint16_t ccs811_errstat = 0; 
uint16_t ccs811_raw = 0;

/******************************************************
 * Function to initialize CCS811
 * use in setup()
 ******************************************************/
void init_ccs811() {
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
}

/******************************************************
 * Function to serial print ccs811 error state
 * use in loop()
 ******************************************************/
void print_ccs811_errstat(uint16_t ccs811_errstat) {
  if (ccs811_errstat == CCS811_ERRSTAT_OK) {
    // all ok
  }
  else if (ccs811_errstat == CCS811_ERRSTAT_OK_NODATA) {
    Serial.println("CCS811: waiting for (new) data");
  }
  else if (ccs811_errstat & CCS811_ERRSTAT_I2CFAIL) { 
    Serial.println("CCS811: I2C error");
  }
  else {
    Serial.print("CCS811: errstat="); 
    Serial.print(ccs811_errstat,HEX); 
    Serial.print("="); 
    Serial.println( ccs811.errstat_str(ccs811_errstat) ); 
  }
}

/******************************************************
 * 2.
 * SDS011 Dust Sensor 
 ******************************************************/
const int SDS_RX_PIN = D7; // D3 -> SDS011 TX pin
const int SDS_TX_PIN = D8; // D4 -> SDS011 RX pin
SoftwareSerial softwareSerial(SDS_RX_PIN, SDS_TX_PIN);
SdsDustSensor sds(softwareSerial); //  additional parameters: retryDelayMs and maxRetriesNotAvailable
float sds011_pm25 = 0;
float sds011_pm10 = 0;

/******************************************************
 * 3.
 * Display
 ******************************************************/
//U8X8_SH1106_128X64_NONAME_HW_I2C display (/* reset=*/ U8X8_PIN_NONE); // display Töötab aga am2320 ei tööta
U8X8_SSD1306_128X64_NONAME_SW_I2C display(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display


/******************************************************
 * 4.
 * BMP280
 ******************************************************/
const int BMP280_ADDR {0x76};
Adafruit_BMP280 bmp;
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();
float bmp280_temp {0};
float bmp280_pressure {0};

/********************************************************
 * Function to initialize BMP280 
 * use in setup() 
 *******************************************************/
void init_bmp280() {

  if (!bmp.begin(BMP280_ADDR)) {
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

/******************************************************************
 * 5.
 * APDS9960
 ******************************************************************/
int proximity {0};  // smaller is closer
int r {0};          // red
int g {0};          // green
int b {0};          // blue

/******************************************************************
 * Function to detect gesture
 * use in loop()
 ******************************************************************/
void gesture_detection() {
  int gesture = APDS.readGesture();

    switch (gesture) {
      case GESTURE_UP:
        Serial.println("-----Detected UP gesture");
        break;

      case GESTURE_DOWN:
        Serial.println("-----Detected DOWN gesture");
        break;

      case GESTURE_LEFT:
        Serial.println("-----Detected LEFT gesture");
        break;

      case GESTURE_RIGHT:
        Serial.println("-----Detected RIGHT gesture");
        break;

      default:
        // ignore
        break;
    }

}

/******************************************************************
 * 6.
 * AM2320
 ******************************************************************/
Adafruit_AM2320 am2320 = Adafruit_AM2320();
float am2320_temp {0};
float am2320_hum {0};


/******************************************************************
 * Timing calculation
 ******************************************************************/
const int T_100MS = 100;
const int ONE_MINUTE = 60000; 
const int FIVE_MINUTES = ONE_MINUTE * 5;
const int FIVE_MINUTES_30SEK = (ONE_MINUTE*5) + (ONE_MINUTE/2); // 5min 30sek

boolean is_100ms = false;
boolean is_1minute = false;
boolean is_5minutes = false;
boolean is_5minutes_30sek = false;

unsigned long prev_100ms_millis = 0;
unsigned long prev_1minute_millis = 0;
unsigned long prev_5minutes_millis = 0;
unsigned long prev_5minutes_30sek = 0;

/******************************************************************
 * Function to calculate time
 * use in loop()
 ******************************************************************/
void calculate_time() {
  // 100ms
  if ((millis() - prev_100ms_millis) >= T_100MS){
    is_100ms = true;
    prev_100ms_millis = millis();
  }
  // 01:00
  if ((millis() - prev_1minute_millis) >= ONE_MINUTE){
    is_1minute = true;
    prev_1minute_millis = millis();
  }
  // 05:00
  if ((millis() - prev_5minutes_millis) >= FIVE_MINUTES){
    is_5minutes = true;
    prev_5minutes_millis = millis();
  }
  // 05:30
  if ((millis() - prev_5minutes_30sek) >= FIVE_MINUTES_30SEK){
    is_5minutes_30sek = true;
    prev_5minutes_30sek = millis();
  }
}


/*****************************************************
 * Functions to Serial print data
 *****************************************************/

void print_sds011() {
  Serial.print("PM2.5 = ");
  Serial.print(sds011_pm25); // float, μg/m3
  Serial.print(", PM10 = ");
  Serial.println(sds011_pm10); // float, μg/m3
}

void print_ccs811() {
  Serial.print("eco2 = ");
  Serial.print(ccs811_eco2);
  Serial.print(" ppm  ");
  Serial.print("etvoc = ");
  Serial.print(ccs811_etvoc);
  Serial.print(" ppb  ");
  Serial.println();
}

void print_bmp280() {
  Serial.print(F("Temperature = "));
  Serial.print(bmp280_temp);
  Serial.println(" *C");

  Serial.print(F("Pressure = "));
  Serial.print(bmp280_pressure);
  Serial.println(" hPa");
}

void print_rgb() {
  Serial.print("PR=");
  Serial.print(proximity);
  Serial.print(" rgb=");
  Serial.print(r);
  Serial.print(",");
  Serial.print(g);
  Serial.print(",");
  Serial.println(b);
}

void print_am2320() {
  Serial.print("AM2320 temp: ");
  Serial.print(am2320_temp);
  Serial.print(" hum: ");
  Serial.println(am2320_hum);
}


/*****************************************************
 * Functions to display data on OLED screen
 *****************************************************/

void display_compile_data() {
  display.setCursor(0, 0);         // Row 0, column 0
  display.print("Compiled:");
  display.setCursor(1, 1);         // Row 1, column 1
  display.print(__DATE__);         // Compile date
  display.setCursor(1, 2);         // Row 2, column 1
  display.print(__TIME__);         // Compile time
  display.setCursor(0, 3);         // Row 3, column 0
  display.print(WiFi.localIP());
  display.setCursor(0, 5);         // Row 5, column 0
  display.print("by Tauno Erik");
}

void display_sds011() {
  // Row 0
  display.setCursor(0, 0);
  display.clearLine(0);
  display.print("PM25:");
  display.setCursor(6, 0);
  display.print(sds011_pm25);
  // Row 1
  display.setCursor(0, 1);
  display.clearLine(1);
  display.print("PM10: ");
  display.setCursor(6, 1);
  display.print(sds011_pm10);
}

void display_ccs811() {
  // Row 2
  display.setCursor(0, 2);
  display.clearLine(2);
  display.print("co2:");
  display.setCursor(6, 2);
  display.print(ccs811_eco2);
  // Row 3
  display.setCursor(0, 3);
  display.clearLine(3);
  display.print("tvoc: ");
  display.setCursor(6, 3);
  display.print(ccs811_etvoc);
}

void display_bmp280() {
  // Row 4
  display.setCursor(0, 4);
  display.clearLine(4);
  display.print("temp: ");
  display.setCursor(6, 4);
  display.print(bmp280_temp);
  // Row 5
  display.setCursor(0, 5);
  display.clearLine(5);
  display.print("pres: ");
  display.setCursor(6, 5);
  display.print(bmp280_pressure);
}

void display_am2320() {
  // Row 6
  display.setCursor(0, 6);
  display.clearLine(6);
  display.print("temp: ");
  display.setCursor(6, 6);
  display.print(am2320_temp);
  // Row 7
  display.setCursor(0, 7);
  display.clearLine(7);
  display.print("hum: ");
  display.setCursor(6, 7);
  display.print(am2320_hum);
}

void display_rgb() {
  // Row 7
  display.clearLine(8);
  display.setCursor(0, 8);
  display.print(r);
  display.setCursor(4, 8);
  display.print(g);
  display.setCursor(8, 8);
  display.print(b);
}


/*********************************************************/


void setup() {
  Serial.begin(115200);
  Wire.begin();

  setup_leds();

  digitalWrite(LED_1, HIGH);
  delay(300);
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, HIGH);
  delay(300);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, HIGH);
  delay(300);
  digitalWrite(LED_3, LOW);
  digitalWrite(LED_4, LOW);
  delay(300);
  digitalWrite(LED_4, HIGH);

  /* Wifi
   ******************************************************/
  Serial.println();
  Serial.printf("Setup: MAC %s\n",WiFi.macAddress().c_str());
  WiFi.mode(WIFI_STA);

  /* Enable ThingSpeak 
   ******************************************************/
  ThingSpeak.begin(client);
  
  /* Display 
   ******************************************************/
  display.begin();
  display.setPowerSave(0);
  display.setFont(u8x8_font_amstrad_cpc_extended_r);

  display_compile_data();
  
  /* SDS011 Dust Sensor 
   ******************************************************/
  sds.begin();
  delay(100);
  // Prints SDS011 firmware version:
  Serial.print("SDS011 ");
  Serial.println(sds.queryFirmwareVersion().toString()); 
  // Ensures SDS011 is in 'query' reporting mode:
  Serial.print("SDS011 ");
  Serial.println(sds.setQueryReportingMode().toString());

  /* CCS811 
   ******************************************************/
  init_ccs811();
  
  /* BMP280
   ****************************************************/
  init_bmp280();
  
  /* APDS9960
   ****************************************************/
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS9960 sensor.");
    while (true); // Stop forever
  }
  
  /* AM2320
   ****************************************************/
  if (!am2320.begin()) {
    Serial.println("Error initializing AM2320 sensor.");
  };

} // setup end


void loop() {

  /* Connect or reconnect to WiFi
   *******************************************************/
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to wifi .");
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(500);     
    } 
    Serial.println("\nConnected.");
  }

  // set the fields with the values
  ThingSpeak.setField(1, sds011_pm25);
  ThingSpeak.setField(2, sds011_pm10);
  ThingSpeak.setField(3, ccs811_eco2);
  ThingSpeak.setField(4, ccs811_etvoc);
  ThingSpeak.setField(5, bmp280_temp);
  ThingSpeak.setField(6, bmp280_pressure);
  ThingSpeak.setField(7, am2320_temp);
  ThingSpeak.setField(8, am2320_hum);

  // set the status
  status_message();
  ThingSpeak.setStatus(myStatus);

  /* APDS9960
   *******************************************************/
  // If a proximity reading is available.
  if (APDS.proximityAvailable()) {
    proximity = APDS.readProximity();
  }
  // If a color reading is available
  if (APDS.colorAvailable()) {
    APDS.readColor(r, g, b);
  }
  // If a gesture reading is available
  if (APDS.gestureAvailable()) {
    gesture_detection();
  }

  /* Calculate the time
  ********************************************************/
  calculate_time();

  /* Do things by time
  ********************************************************/
  if (is_100ms) {
    is_100ms = false;
  }

  // Do this every 1 minutes
  if (is_1minute){
    // SDS011
    Serial.println();
    print_sds011();
    display_sds011();

    // AM2320
    am2320_temp = am2320.readTemperature();
    am2320_hum = am2320.readHumidity();
    print_am2320();
    display_am2320();
    
    // CCS811
    ccs811.read(&ccs811_eco2,&ccs811_etvoc,&ccs811_errstat,&ccs811_raw);
    print_ccs811_errstat(ccs811_errstat);
    print_ccs811();
    display_ccs811();

    // BMP280
    sensors_event_t temp_event, pressure_event;
    bmp_temp->getEvent(&temp_event);
    bmp_pressure->getEvent(&pressure_event);
    bmp280_temp = temp_event.temperature;
    bmp280_pressure = pressure_event.pressure;
    print_bmp280();
    display_bmp280();

    // APDS9960
    print_rgb();
    //display_rgb(); // no room on display
    
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

    // write to the ThingSpeak channel
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if(x == 200){
      Serial.println("ThingSpeak update successful.");
    }
    else{
      Serial.print("Problem updating ThingSpeak.");
      Serial.println(" HTTP error code " + String(x));
    }

    is_5minutes_30sek = false;
  }

} // main loop end