# tauno-env-monitor
Home environment monitoring system: dust, co2, humidity, temp

## Main board: WeMos D1 mini (ESP8266)

- 3.3V

Default I2C SDA/SCL pins:

- D2 - GPIO4 - SDA
- D1 - GPIO5 - SCL

## BMP280 Humidity, temperature, barometric pressure and altitude sensor

- 3.3V
- I2C Address: 0x76 or 0x77
- Adafruit_BMP280_Library
- https://learn.adafruit.com/adafruit-bmp280-barometric-pressure-plus-temperature-sensor-breakout/overview

## APDS-9960 Proximity, RGB and gesture sensor

- 3.3V
- I2C Address: 0x39
- ambient light
- color measuring
- proximity detection
- touchless gesture. Operating range 10-20cm.

Arduino library:

- It works when you comment out //_wire.end();
- https://github.com/arduino-libraries/Arduino_APDS9960

## CCS-811 (CJMCU-811) Digital gas sensor monitoring indoor air quality

I2C Address: 0x5A / 0x5B

- 3.3V
- needs pullup resistors
- CO2
- TVOCs

Update sensor firmware:

- https://github.com/maarten-pennings/CCS811/tree/master/examples/ccs811flash

Library: 

- https://github.com/adafruit/Adafruit_CCS811
- https://github.com/maarten-pennings/CCS811

## 1.3" Oled display module 128x32

I2C Address: 

### Links:

- https://learn.adafruit.com/monochrome-oled-breakouts

------------------------------------

## AM2320 Temperature and humidity sensor

I2C Address: 0x5C

- 3.3V

### Links:

- https://learn.adafruit.com/adafruit-am2320-temperature-humidity-i2c-sensor

## SDS011 Air quality (air particles or dust) sensor

- 5V
- RX UART data (3.3V)
- TX UART data (3.3V)

### Links:

- https://github.com/ricki-z/SDS011

Python scripts (not tested):

- https://gist.github.com/netmaniac/a6414149a5a09ba1ebf702ff8d5056c5
- https://gitlab.com/frankrich/sds011_particle_sensor