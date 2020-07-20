/*****************************************************
 * File: main.cpp
 * By Tauno Erik
 * Modified: 20. juuli 2020
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
//#include <Adafruit_Sensor.h>
//#include <Adafruit_AM2320.h>
#include "AM2320.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
//#include <ESP8266WiFiMulti.h>
#include <ThingSpeak.h>
#include "secrets.h"
#include "index.h"
// File secrets.h content:
// #define SECRET_SSID "MySSID"     // replace MySSID with your WiFi network name
// #define SECRET_PASS "MyPassword" // replace MyPassword with your WiFi password
// Thingspeak

/*
 TODO:
 1. status messages
 2. LEDs
 3. Web interface
*/

/********************
 * Constants
 ********************/
const int LED_1 = D6;
const int LED_2 = D5;
const int LED_3 = D0;
const int LED_4 = D4;       // allso on board LED!
const int NWAKE_PIN = D3;   // nWAKE on D3 or GND
const int SDS_RX_PIN = D7;  // D7 -> SDS011 TX pin
const int SDS_TX_PIN = D8;  // D8 -> SDS011 RX pin

// Wifi & Thingspeak:
const char * ssid = SECRET_SSID_1;   // your network SSID (name) 
const char * pass = SECRET_PASS_1;   // your network password
const unsigned long myChannelNumber = THINGSPEAK_CH_ID;
const char * myWriteAPIKey = THINGSPEAK_WRITE_APIKEY;

const int BMP280_ADDR = 0x76;

const int TWO_SECONDS = 2000; // 2 secons
const int ONE_MINUTE = 60000; 
const int FIVE_MINUTES = ONE_MINUTE * 5;
const int FIVE_MINUTES_30SEK = (ONE_MINUTE*5) + (ONE_MINUTE/2); // 5min 30sek

/********************
 * Global variables
 ********************/

//CCS811 co2 sensor:
uint16_t ccs811_eco2 = 0; 
uint16_t ccs811_etvoc = 0;
uint16_t ccs811_errstat = 0; 
uint16_t ccs811_raw = 0;

float sds011_pm25 = 0;
float sds011_pm10 = 0;

float bmp280_temp = 0;
float bmp280_pressure = 0;

int proximity = 0;  // smaller is closer //APDS9960
int r = 0;          // red
int g = 0;          // green
int b = 0;          // blue

float am2320_temp = 0;
float am2320_hum = 0;
int am2320_errstat = 0;

boolean is_first_loop = true;
boolean is_2seconds = false;
boolean is_1minute = false;
boolean is_5minutes = false;
boolean is_5minutes_30sek = false;

unsigned long prev_2seconds_millis = 0;
unsigned long prev_1minute_millis = 0;
unsigned long prev_5minutes_millis = 0;
unsigned long prev_5minutes_30sek = 0;


/********************
 * Object Calls
 *******************/
WiFiClient  client;
ESP8266WebServer server(80);

CCS811 ccs811(NWAKE_PIN);            

SoftwareSerial softwareSerial(SDS_RX_PIN, SDS_TX_PIN);
SdsDustSensor sds(softwareSerial); //  additional parameters: retryDelayMs and maxRetriesNotAvailable

//U8X8_SH1106_128X64_NONAME_HW_I2C display (/* reset=*/ U8X8_PIN_NONE); // display Töötab aga am2320 ei tööta
U8X8_SSD1306_128X64_NONAME_SW_I2C display(SCL, SDA, U8X8_PIN_NONE);

Adafruit_BMP280 bmp;
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

//Adafruit_AM2320 am2320 = Adafruit_AM2320();
AM2320 am2320(&Wire);


/* Functions Prototyps
****************************************/
void setup_leds();
void init_ccs811();
void print_ccs811_errstat(uint16_t ccs811_errstat);
String get_status();
void init_bmp280();
void gesture_detection();
void calculate_time();
void read_sds011();
void read_am2320();
void serial_print_sensors_data();
void set_thingspeak_fields();
void display_compile_data();
void display_sds011();
void display_data();
void display_rgb();

void web_root();
void web_404();
void web_sds011_pm25();
void web_sds011_pm10();
void web_ccs811_eco2();
void web_ccs811_etvoc();
void web_bmp280_temp();
void web_bmp280_pressure();
void web_am2320_temp();
void web_am2320_hum();
void web_proximity();
void web_r();
void web_g();
void web_b();

/*********************************************************/

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA,SCL);

  setup_leds();

  /* Wifi
   ******************************************************/
  Serial.printf("\nSetup: MAC %s\n",WiFi.macAddress().c_str());
  WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, pass);
  Serial.print(WiFi.localIP());
  Serial.println();

  server.on("/", web_root);
  server.onNotFound(web_404);
  server.on("/read_sds011_pm25", web_sds011_pm25);
  server.on("/read_sds011_pm10", web_sds011_pm10);
  server.on("/read_ccs811_eco2", web_ccs811_eco2);
  server.on("/read_ccs811_etvoc", web_ccs811_etvoc);
  server.on("/read_bmp280_temp", web_bmp280_temp);
  server.on("/read_bmp280_pressure", web_bmp280_pressure);
  server.on("/read_am2320_temp", web_am2320_temp);
  server.on("/read_am2320_hum", web_am2320_hum);
  server.on("/read_proximity", web_proximity);
  server.on("/read_r", web_r);
  server.on("/read_g", web_g);
  server.on("/read_b", web_b);
  
  server.begin();

  /* Enable ThingSpeak 
   ******************************************************/
  ThingSpeak.begin(client);
  
  /* Display 
   ******************************************************/
  display.begin();
  display.setPowerSave(0);
  //display.setFont(u8x8_font_amstrad_cpc_extended_r);
  display.setFont(u8x8_font_amstrad_cpc_extended_f);
  //display.setFont(u8x8_font_chroma48medium8_r);
  
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


  init_ccs811();
  
  init_bmp280();
  
  /* APDS9960
   ****************************************************/
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS9960 sensor.");
    // while (true); // Stop forever
  }
  
  /* AM2320
   ****************************************************/
  /*if (!am2320.begin()) {
    Serial.println("Error initializing AM2320 sensor.");
  };*/

  display_compile_data();

} // setup end


void loop() {

  /* Connect or reconnect to WiFi
   *******************************************************/
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to wifi .");
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);
      Serial.print(".");
      delay(500);     
    } 
    Serial.println("\n Wifi Connected.");
  }

  server.handleClient(); // display webpage


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


  calculate_time();

  /* Do things by time
  ********************************************************/
  if (is_2seconds) {
    // Read CCS811
    ccs811.read(&ccs811_eco2,&ccs811_etvoc,&ccs811_errstat,&ccs811_raw);
    print_ccs811_errstat(ccs811_errstat);
    // AM2320
    read_am2320();
    // BMP280
    sensors_event_t temp_event, pressure_event;
    bmp_temp->getEvent(&temp_event);
    bmp_pressure->getEvent(&pressure_event);
    bmp280_temp = temp_event.temperature;
    bmp280_pressure = pressure_event.pressure;

    is_2seconds = false;
  }

  
  if (is_1minute){
    // AM2320
    //am2320_temp = am2320.readTemperature();
    //am2320_hum = am2320.readHumidity();
    /*switch(am2320.Read()) {
      case 2:
        Serial.println(F("  CRC failed"));
        break;
      case 1:
        Serial.println(F("  Sensor offline"));
        break;
      case 0:
        am2320_hum = am2320.Humidity;
        am2320_temp = am2320.cTemp;
        break;
    }*/

    // BMP280
    /*sensors_event_t temp_event, pressure_event;
    bmp_temp->getEvent(&temp_event);
    bmp_pressure->getEvent(&pressure_event);
    bmp280_temp = temp_event.temperature;
    bmp280_pressure = pressure_event.pressure;*/
    
    display_data();
    serial_print_sensors_data();
    
    is_1minute = false;
  }

  
  if (is_5minutes){
    sds.wakeup();
    is_5minutes = false;
  }

  
  if (is_5minutes_30sek){
    read_sds011();
    display_sds011();

    set_thingspeak_fields(); 
    ThingSpeak.setStatus(get_status());

    // Write to the ThingSpeak channel
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

/********************************************************************/

/**
 * Function to read data from SDS011
 * Afterwards puts back to sleep
 **/
void read_sds011() {
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
}

/**
 * Function to initialize LEDs pins and intro blinks.
 * Use in loop()
 **/
void setup_leds() {
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_3, OUTPUT);
  pinMode(LED_4, OUTPUT);

  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, LOW);
  digitalWrite(LED_4, HIGH); // HIGH = on board led off

  digitalWrite(LED_1, HIGH);
  delay(500);
  digitalWrite(LED_1, LOW);
  digitalWrite(LED_2, HIGH);
  delay(500);
  digitalWrite(LED_2, LOW);
  digitalWrite(LED_3, HIGH);
  delay(500);
  digitalWrite(LED_3, LOW);
  digitalWrite(LED_4, LOW);
  delay(500);
  digitalWrite(LED_4, HIGH);
}


/**
 * Function to initialize CCS811
 * Use in setup()
 **/
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


/**
 * Function to serial print ccs811 error state
 * use in loop()
 **/
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


/**
 * Function to set ThingSpeak the fields with the values
 **/
void set_thingspeak_fields() {
  ThingSpeak.setField(1, sds011_pm25);
  ThingSpeak.setField(2, sds011_pm10);
  ThingSpeak.setField(3, ccs811_eco2);
  ThingSpeak.setField(4, ccs811_etvoc);
  ThingSpeak.setField(5, bmp280_temp);
  ThingSpeak.setField(6, bmp280_pressure);
  ThingSpeak.setField(7, am2320_temp);
  ThingSpeak.setField(8, am2320_hum);
}

/**
 * Function to generate ThingSpeaks status messages
 **/
String get_status() {
  String status = ""; 

  if (am2320_errstat == 2){
    status += String("-am2320er");
  }
  else if (ccs811_errstat != CCS811_ERRSTAT_OK) {
    status += String("-ccs811er");
  }
  else{
    status += String("-ok");
  }
  
  return status;
}


/**
 * Function to initialize BMP280 
 * Use in setup() 
 **/
void init_bmp280() {

  if (!bmp.begin(BMP280_ADDR)) {
    Serial.println(F("Could not find a BMP280 sensor, check wiring or I2C!"));
    // while (1) delay(10); // Stop forever
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  bmp_temp->printSensorDetails();
}


/**
 * 
 **/
void read_am2320() {
  am2320_errstat = am2320.Read();
  if (am2320_errstat == 0) {
    am2320_hum = am2320.Humidity;
    am2320_temp = am2320.cTemp;
  }
  /*switch(am2320.Read()) {
      case 2:
        Serial.println(F("  CRC failed"));
        break;
      case 1:
        Serial.println(F("  Sensor offline"));
        break;
      case 0:
        am2320_hum = am2320.Humidity;
        am2320_temp = am2320.cTemp;
        break;
    }*/
}


/**
 * Function to detect gesture
 * Use in loop()
 **/
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


/**
 * Function to calculate timing times
 * Use in loop()
 **/
void calculate_time() {
  // 100ms
  if ((millis() - prev_2seconds_millis) >= TWO_SECONDS){
    is_2seconds = true;
    prev_2seconds_millis= millis();
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
void serial_print_sensors_data(){
  Serial.printf("\nPM2.5: %.2f μg/m3", sds011_pm25);
  Serial.printf(", PM10: %.2f μg/m3\n", sds011_pm10);

  Serial.printf("eCO2: %i ppm,", ccs811_eco2);
  Serial.printf(" eTVOC: %i ppb\n", ccs811_etvoc);

  Serial.printf("BMP Temp: %.2f *C,", bmp280_temp);
  Serial.printf(" Pressure = %.2f hPa\n", bmp280_pressure);

  Serial.printf("AM2 temp: %.2f *C,", am2320_temp);
  Serial.printf(" Hum: %.2f\n", am2320_hum);

  Serial.printf("PR: %i ", proximity);
  Serial.printf(" rgb: %i,%i,%i\n", r,g,b);
}

/*****************************************************
 * Functions to display data on OLED screen
 *****************************************************/

void display_compile_data() {
  display.drawString(0, 0,"compiled:");
  display.drawString(1, 1, __DATE__);
  display.drawString(1, 2, __TIME__);
  display.setCursor(0, 3);
  display.print(WiFi.localIP());
  display.drawString(0, 5, "by Tauno Erik");
}

void display_sds011() {
  // Row 0
  display.clearLine(0);
  display.drawString(0, 0,"PM25:");
  display.setCursor(6, 0);
  display.print(sds011_pm25);
  // Row 1
  display.clearLine(1);
  display.drawString(0, 1,"PM10:");
  display.setCursor(6, 1);
  display.print(sds011_pm10);
}

void display_data() {
  if (is_first_loop) {
    display.clearLine(0);
    display.drawString(0, 0,"PM25:");
    display.clearLine(1);
    display.drawString(0, 1,"PM10:");
    is_first_loop = false;
  }
  // Row 2
  display.clearLine(2);
  display.drawString(0, 2,"co2:");
  display.setCursor(6, 2);
  display.print(ccs811_eco2);
  // Row 3
  display.clearLine(3);
  display.drawString(0, 3,"tvoc:");
  display.setCursor(6, 3);
  display.print(ccs811_etvoc);
  // Row 4
  display.clearLine(4);
  display.drawString(0, 4,"temp:");
  display.setCursor(6, 4);
  display.print(bmp280_temp);
  // Row 5
  display.clearLine(5);
  display.drawString(0, 5,"pres:");
  display.setCursor(6, 5);
  display.print(bmp280_pressure);
  // Row 6
  display.clearLine(6);
  display.drawString(0, 6,"temp:");
  display.setCursor(6, 6);
  display.print(am2320_temp);
  // Row 7
  display.clearLine(7);
  display.drawString(0, 7,"hum:");
  display.setCursor(6, 7);
  display.print(am2320_hum);
}

void display_rgb() {
  // No room on display
  display.clearLine(8);
  display.setCursor(0, 8);
  display.print(r);
  display.setCursor(4, 8);
  display.print(g);
  display.setCursor(8, 8);
  display.print(b);
}

/**
 * Web pages
 **/

void web_404(){
  server.send(404, "text/plain", "404: Not found");
}

void web_root() {
  server.send_P(200, "text/html", MAIN_page, sizeof(MAIN_page));
}

void web_sds011_pm25(){
  String a = String(sds011_pm25);
  server.send(200, "text/plane", a);
}

void web_sds011_pm10(){
  String a = String(sds011_pm10);
  server.send(200, "text/plane", a);
}

void web_ccs811_eco2(){
  String a = String(ccs811_eco2);
  server.send(200, "text/plane", a);
}

void web_ccs811_etvoc() {
  String a = String(ccs811_etvoc);
  server.send(200, "text/plane", a);
}

void web_bmp280_temp() {
  String a = String(bmp280_temp);
  server.send(200, "text/plane", a);
}

void web_bmp280_pressure() {
  String a = String(bmp280_pressure);
  server.send(200, "text/plane", a);
}

void web_am2320_temp() {
  String a = String(am2320_temp);
  server.send(200, "text/plane", a);
}

void web_am2320_hum() {
  String a = String(am2320_hum);
  server.send(200, "text/plane", a);
}

void web_proximity() {
  String a = String(proximity);
  server.send(200, "text/plane", a);
}

void web_r() {
  String a = String(r);
  server.send(200, "text/plane", a);
}

void web_g(){
  String a = String(g);
  server.send(200, "text/plane", a);
}

void web_b(){
  String a = String(b);
  server.send(200, "text/plane", a);
}