# js_BME680_Library wihout BSEC delivering IAQ by hermannj's iaq aquiring algoritm 
The BME680 precision sensor from Bosch SensorTec can measure humidity with ±3% accuracy, barometric pressure with ±1 hPa absolute accuracy, and temperature with ±1.0°C accuracy. 
Because pressure changes with altitude, and the pressure measurements are so good, you can also use it as an altimeter with  ±1 meter or better accuracy!
The BME680 takes those sensors to the next step in that it contains a small MOX sensor. The heated metal oxide changes resistance based on the volatile organic compounds (VOC) in the air, 
so it can be used to detect gasses & alcohols such as Ethanol, Alcohol and Carbon Monoxide, and perform air quality measurements. 
Note it will give you one resistance value, with overall VOC content, but it cannot differentiate gasses or alcohols.
Based on herrmannj's algorithm IAQ in PPM-Units  could be delivered without using proprietary BSEC static libs. 
*******************************************************************************************************************************
Credits and very thanks to Jörg Herrmann @ https://github.com/herrmannj/ and https://forum.fhem.de/index.php/topic,96241.0.html
Author: js @ https://github.com/juergs and https://forum.fhem.de/index.php/topic,78619.0.html
*******************************************************************************************************************************

Designed specifically to work with the Adafruit BME680 Breakout  * http://www.adafruit.com/products/3660 and libraries.  

These sensors use I2C to communicate, up to 4 pins are required to interface

<img src="https://cdn-shop.adafruit.com/970x728/3660-00.jpg" height="300"/>
# Adafruit BME680 Library [![Build Status](https://travis-ci.org/adafruit/Adafruit_BME680.svg?branch=master)](https://travis-ci.org/adafruit/Adafruit_BME680)
Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Check out the links above for our tutorials and wiring diagrams

On-line documention for the APIs is available, too:
  * https://adafruit.github.io/Adafruit_BME680/html/index.html

Requires installation of the Adafruit Unified Sensor library:
  * https://github.com/adafruit/Adafruit_Sensor

Written by Limor Fried/Ladyada for Adafruit Industries.
BSD license, all text above must be included in any redistribution

****************************************************************************************************************************
Serial-Monitor output @ 115KBd
```
*** Started!
Enabled: I2C for Wemos D1 mini SDA:D2 / SCL:D1
Scanning I2C...
I2C device found at address 0x77  !
done

I2C: ok BME680 sensor found! :-)
[10.37]   Temperature = 27.32 *C  Pressure = 999.40 hPa  Humidity = 51.94 %  Gas = 0.00 KOhms  Approx. Altitude = 115.79 m 
[20.37]   Temperature = 27.41 *C  Pressure = 999.38 hPa  Humidity = 51.92 %  Gas = 87.76 KOhms  Approx. Altitude = 115.79 m 
[30.37]   Temperature = 27.53 *C  Pressure = 999.40 hPa  Humidity = 51.85 %  Gas = 95.60 KOhms  Approx. Altitude = 116.12 m 
[40.37]   Temperature = 27.59 *C  Pressure = 999.40 hPa  Humidity = 51.88 %  Gas = 100.18 KOhms  Approx. Altitude = 115.95 m 
[50.37]   Temperature = 27.63 *C  Pressure = 999.40 hPa  Humidity = 51.94 %  Gas = 102.70 KOhms  Approx. Altitude = 115.79 m 
[60.37]   Temperature = 27.64 *C  Pressure = 999.40 hPa  Humidity = 51.99 %  Gas = 104.36 KOhms  Approx. Altitude = 115.95 m 
[70.37]   Temperature = 27.65 *C  Pressure = 999.38 hPa  Humidity = 51.99 %  Gas = 105.15 KOhms  Approx. Altitude = 115.95 m 
[80.38]   Temperature = 27.66 *C  Pressure = 999.38 hPa  Humidity = 52.02 %  Gas = 106.08 KOhms  Approx. Altitude = 115.95 m 
[90.38]   Temperature = 27.66 *C  Pressure = 999.38 hPa  Humidity = 52.02 %  Gas = 106.21 KOhms  Approx. Altitude = 115.95 m 
[100.38]   Temperature = 27.66 *C  Pressure = 999.40 hPa  Humidity = 51.98 %  Gas = 107.30 KOhms  Approx. Altitude = 116.12 m 
[110.38]   Temperature = 27.66 *C  Pressure = 999.38 hPa  Humidity = 51.93 %  Gas = 108.13 KOhms  Approx. Altitude = 116.12 m 
[120.38]   Temperature = 27.67 *C  Pressure = 999.36 hPa  Humidity = 51.83 %  Gas = 108.48 KOhms  Approx. Altitude = 116.29 m 
[130.38]   Temperature = 27.68 *C  Pressure = 999.36 hPa  Humidity = 51.78 %  Gas = 108.41 KOhms  Approx. Altitude = 116.12 m 
[140.38]   Temperature = 27.68 *C  Pressure = 999.36 hPa  Humidity = 51.83 %  Gas = 108.84 KOhms  Approx. Altitude = 116.12 m 
[150.38]   Temperature = 27.68 *C  Pressure = 999.38 hPa  Humidity = 51.89 %  Gas = 108.62 KOhms  Approx. Altitude = 116.29 m 
[160.38]   Temperature = 27.69 *C  Pressure = 999.36 hPa  Humidity = 51.86 %  Gas = 108.70 KOhms  Approx. Altitude = 116.46 m 
[170.38]   Temperature = 27.69 *C  Pressure = 999.34 hPa  Humidity = 51.84 %  Gas = 109.48 KOhms  Approx. Altitude = 116.46 m 
[180.39]   Temperature = 27.70 *C  Pressure = 999.36 hPa  Humidity = 51.83 %  Gas = 109.26 KOhms  Approx. Altitude = 116.12 m 
[190.39]   Temperature = 27.69 *C  Pressure = 999.36 hPa  Humidity = 51.85 %  Gas = 109.41 KOhms  Approx. Altitude = 116.29 m 
[200.39]   Temperature = 27.69 *C  Pressure = 999.36 hPa  Humidity = 51.83 %  Gas = 109.33 KOhms  Approx. Altitude = 116.29 m 
[210.39]   Temperature = 27.68 *C  Pressure = 999.36 hPa  Humidity = 51.83 %  Gas = 109.48 KOhms  Approx. Altitude = 116.29 m 
[220.39]   Temperature = 27.68 *C  Pressure = 999.32 hPa  Humidity = 51.84 %  Gas = 110.05 KOhms  Approx. Altitude = 116.46 m 
[230.39]   Temperature = 27.68 *C  Pressure = 999.32 hPa  Humidity = 51.82 %  Gas = 110.34 KOhms  Approx. Altitude = 116.29 m 
[240.39]   Temperature = 27.68 *C  Pressure = 999.32 hPa  Humidity = 51.80 %  Gas = 111.00 KOhms  Approx. Altitude = 116.29 m 
[250.39]   Temperature = 27.68 *C  Pressure = 999.32 hPa  Humidity = 51.78 %  Gas = 110.93 KOhms  Approx. Altitude = 116.12 m 
[260.39]   Temperature = 27.68 *C  Pressure = 999.34 hPa  Humidity = 51.81 %  Gas = 110.93 KOhms  Approx. Altitude = 116.29 m 
[270.39]   Temperature = 27.68 *C  Pressure = 999.32 hPa  Humidity = 51.80 %  Gas = 110.63 KOhms  Approx. Altitude = 116.46 m 
[280.40]   Temperature = 27.68 *C  Pressure = 999.34 hPa  Humidity = 51.75 %  Gas = 111.52 KOhms  Approx. Altitude = 116.46 m 
[290.40]   Temperature = 27.68 *C  Pressure = 999.34 hPa  Humidity = 51.80 %  Gas = 110.78 KOhms  Approx. Altitude = 116.46 m 
[300.40]   Temperature = 27.69 *C  Pressure = 999.34 hPa  Humidity = 51.77 %  Gas = 111.37 KOhms  Approx. Altitude = 116.63 m 
[300.77] Taupunkt: 17.00  tVOC: 125
BME680-Reading done.
[310.40]   Temperature = 27.69 *C  Pressure = 999.32 hPa  Humidity = 51.78 %  Gas = 111.37 KOhms  Approx. Altitude = 116.46 m 
[310.77] Taupunkt: 17.01  tVOC: 125
BME680-Reading done.
[320.40]   Temperature = 27.70 *C  Pressure = 999.30 hPa  Humidity = 51.76 %  Gas = 111.96 KOhms  Approx. Altitude = 116.63 m 
[320.77] Taupunkt: 17.00  tVOC: 125
BME680-Reading done.
[330.40]   Temperature = 27.70 *C  Pressure = 999.30 hPa  Humidity = 51.78 %  Gas = 111.07 KOhms  Approx. Altitude = 116.46 m 
[330.77] Taupunkt: 17.00  tVOC: 125
BME680-Reading done.
[340.40]   Temperature = 27.70 *C  Pressure = 999.34 hPa  Humidity = 51.77 %  Gas = 111.82 KOhms  Approx. Altitude = 116.46 m 
[340.77] Taupunkt: 17.00  tVOC: 125
BME680-Reading done.
[350.40]   Temperature = 27.70 *C  Pressure = 999.34 hPa  Humidity = 51.79 %  Gas = 111.44 KOhms  Approx. Altitude = 116.46 m 
[350.78] Taupunkt: 17.02  tVOC: 126
BME680-Reading done.
```