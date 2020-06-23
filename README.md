# tauno-env-monitor
Home environment monitoring system: dust, co2, humidity, temp

## Main board: WeMos D1 mini

-3.3V

## SDS011 Air quality (air particles or dust) sensor

- 5V
- RX UART data (3.3V)
- TX UART data (3.3V)

### Links:

- https://github.com/ricki-z/SDS011

Python scripts:

- https://gist.github.com/netmaniac/a6414149a5a09ba1ebf702ff8d5056c5
- https://gitlab.com/frankrich/sds011_particle_sensor

## BMP280 Barometric pressure and altitude sensor

I2C Addr: 

- 3.3V

### Links:

- https://learn.adafruit.com/adafruit-bmp280-barometric-pressure-plus-temperature-sensor-breakout/overview

## APDS-9960 RGB and gesture sensor

I2C Addr: 0x39

- 3.3V
- ambient light
- color measuring
- proximity detection
- touchless gesture. Operating range 10-20cm. 

### Links:

- https://github.com/sparkfun/SparkFun_APDS-9960_Sensor_Arduino_Library
- https://github.com/sparkfun/APDS-9960_RGB_and_Gesture_Sensor
- https://learn.adafruit.com/adafruit-apds9960-breakout

## AM2320 Temperature and humidity sensor

I2C Addr: 0x5C

- 3.3V

### Links:

- https://learn.adafruit.com/adafruit-am2320-temperature-humidity-i2c-sensor

## 1.3" Oled display module 128x32

I2C Addr:

### Links:

- https://learn.adafruit.com/monochrome-oled-breakouts

## CCS-811 (CJMCU-811) Digital gas sensor, CO2, TVOCs sensor

I2C Addr: 0x5A / 0x5B

- 3.3V

### Links: 

- https://github.com/sciosense/CCS811_driver
- https://github.com/maarten-pennings/CCS811