# My home environment and air quality monitoring system

![Freeform environment system](./img/IMG_7979.jpg)

We often hide the electronics in a box. And cases can be done well and nicely. But when our main interest is electronics then it is sad to hide it. That is way like to do it the opposite way.

Sensors will measure particulate matter (Ultrafine dust PM2.5 and fine dust PM10), co2, TVOC, ambient light, humidity, temperature and barometric pressure.

Most sensors use I2C to communicate. That makes it super easy to build.

## Why and what

We spend much of our lives indoors. And that is why it is important that our main living environment is healthy.

- CO2 level should be less than <1000 ppm
- TVOC is the concentration of Volatile organic compounds (VOCs) present in the air. Should be less than <100 ppm.
- PM2.5 refers to atmospheric particulate matter (PM) that have a diameter of less than 2.5 micrometers. The 24-hour concentration of PM2.5 is considered unhealthy when it rises above 35.4 (60) μg/m3.
- PM10 refers to atmospheric particulate matter (PM) that have a diameter of less than 10 micrometers. < 100 μg/m3.
- Humidity 40-60 %
- Temperature 21-25 °C

## Schemetics

This is the basic schematics.

![Basic](./Schematics/basic.jpg)

Because I still had free pins, I added four LEDs.

![Basic+LEDs](./Schematics/basic-leds.jpg)

My power supply is 9V. This is a circuit to make 3.3C and 5V what my MCU and sensors need.

![Power](./Schematics/power.jpg)

## Data

There are four places to view real-time data:

- On display
- Via serial monitor
- Local webpage
- ThingSpeak channel. Publick view: https://thingspeak.com/channels/1099844

In the long term, all data is stored on ThingSpeak channel.

Serial Monitor view:
![Website](./doc/serial.jpg)

Simple local website:
![Website](./doc/Ekraan_web.jpg)

ThingSpeak view:
![Website](./doc/ThingSpeak.jpg)

### Notes

- First test all sensors separately to make sure they all work properly.
- There are different libraries. Test them to be sure they will work with your MCU and sensor.
- Keep code modular. So you can easily add and remove sections.
- Wear gloves. The wires may become hot during soldering.

### Links:

- https://thingspeak.com/
- https://github.com/mathworks/thingspeak-arduino

## Main board: WeMos D1 mini (ESP8266)

![Wemos D1 mini](./doc/wemos-d1-mini.jpg)

- 3.3V

Default I2C SDA/SCL pins:

- D2 - GPIO4 - SDA
- D1 - GPIO5 - SCL

### Links:

- https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/

## BMP280 Humidity, temperature, barometric pressure and altitude sensor

![BMP280](./doc/BMP280.jpg)

- I2C Address: 0x76 or 0x77

### Pins

|  BMP280  |  ESP8266  |
|:--------:|:---------:|
|   VCC    |    3V3    |
|   GND    |    GND    |
|   SCL    |    D1     |
|   SDA    |    D2     |
|   CSE    |    -      |
|   SDD    |    -      |

### Library:

- https://github.com/adafruit/Adafruit_BMP280_Library

## APDS-9960 Proximity, RGB and gesture sensor

![APDS-9960](./doc/apds9960.jpg)

- I2C Address: 0x39
- ambient light
- color measuring
- proximity detection
- touchless gesture. Operating range 10-20cm.

### Pins

| APDS9960 |  ESP8266  |
|:--------:|:---------:|
|   VIN    |    3V3    |
|   GND    |    GND    |
|   SCL    |    D1     |
|   SDA    |    D2     |
|   INT    |    -      |

### Library:

- It works when you comment out //_wire.end();
- https://github.com/arduino-libraries/Arduino_APDS9960

## CCS-811 (CJMCU-811) Digital gas sensor monitoring indoor air quality

![CCS-811](./doc/ccs811.jpg)

- I2C Address: 0x5A or 0x5B
- Needs pullup resistors
- eCO2
- eTVOCs

### Pins

| CCS811  |  ESP8266  |
|:-------:|:---------:|
|   VCC   |    3V3    |
|   GND   |    GND    |
|   SDA   |    D2     |
|   SCL   |    D1     |
| nWAKE   | D3 or GND |
|   INT   |    -      |
|   RST   |    -      |
|   ADD   |    -      |

### Update sensor firmware:

- https://github.com/maarten-pennings/CCS811/tree/master/examples/ccs811flash

### Library: 

- https://github.com/adafruit/Adafruit_CCS811
- https://github.com/maarten-pennings/CCS811

## 1.3" Oled display module 128x64

![OLED](./doc/oled.jpg)

- I2C Address: 0x3c (Although marking on display says 0x78).

Unknown manufacturer. After trial and error, I discovered my display driver is SH1106. Not SSD1306 like Adafruit uses. 

But when I use SH1106 constructor:

    U8X8_SH1106_128X64_NONAME_HW_I2C display (/* reset=*/ U8X8_PIN_NONE);

Display will looks good. But the AM2320 sensor will not work for unknown reason. I2C communication problem?

Thats way I still use SSD1306 constructor.The fonts does not look nice and on the screen right side has white scratched strip.

    U8X8_SSD1306_128X64_NONAME_SW_I2C display(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);

### Pins

|   OLED  |  ESP8266  |
|:-------:|:---------:|
|   GND   |    GND    |
|   VCC   |    5V     |
|   SCL   |    D1     |
|   SDA   |    D2     |

### Library:

- https://github.com/olikraus/U8g2_Arduino
- https://github.com/olikraus/u8g2/wiki/u8x8reference

------------------------------------

## AM2320 Temperature and humidity sensor

![AM2320](./doc/AM2320.jpg)

- I2C Address: 0x5C

## PROBLEM!

Sensor will work normally a couple of hours. After that will not respond anymore. Needs power off restart.

### Pins

| AM2320  |  ESP8266  |
|:-------:|:---------:|
|   GND   |    GND    |
|   VCC   |    3V3    |
|   SDA   |    D2     |
|   SCL   |    D1     |

### Library:

- https://github.com/EngDial/AM2320
- https://github.com/adafruit/Adafruit_AM2320 (works 2h then returns nan)

## Nova PM SDS011 Dust sensor

![SDS011](./doc/SDS011.jpg)

Can measure fine dust and smoke = particulate matter (MP) concentrations in two categories:

- Ultrafine dust particles with a diameter of 0 – 2.5 micrometres (μm/m3). Output PM2.5
- Fine dust particles with a diameter 2.5 – 10 micrometres (μm/m3). Output PM10
- RX UART data (3.3V)
- TX UART data (3.3V)

### Pins

| SDS011  |  ESP8266  |
|:-------:|:---------:|
|   TXD   |     D7    |
|   RXD   |     D8    |
|   GND   |    GND    |
|   25um  |     -     |
|   5V    |     5V    |
|   1um   |     -     |
|   NC    |     -     |

### Library:

- https://github.com/lewapek/sds-dust-sensors-arduino-library

### Python scripts (not tested):

- https://gist.github.com/netmaniac/a6414149a5a09ba1ebf702ff8d5056c5
- https://gitlab.com/frankrich/sds011_particle_sensor

___

Copyright 2021 Tauno Erik https://taunoerik.art
