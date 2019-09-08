# ESPEasy_BME680_TVOC
<h2>ESPEasy_BME680_TVOC without need of proprietary BSEC static lib </h2>

These additional bold libraries are needed for using BME680-Sensor in ESPEasy **\lib**-folder.:

**Adafruit_BME680_Library**
<br>**Adafruit_Unified_Sensor**
<br>**js_BME680_prod**
<br>**Adafruit_Sensor**

<u>These were part of the regular distribution:</u>
<br>Adafruit_Motor_Shield_V2
<br>Adafruit_MPR121
<br>Adafruit_NeoPixel
<br>Adafruit_SGP30-1.0.0.13
<br>Adafruit_TCS34725
<br>Adafruit_TSL2591
<br>AM2320
<br>ArduinoJson-6.x
<br>ArduinoTrace
<br>AS_BH1750
<br>Blynk
<br>ccronexpr
<br>ESP32_ping
<br>esp8266-oled-ssd1306
<br>ESPEasySerial
<br>ESPEasy_ESP8266Ping
<br>HLW8012_1.1.1
<br>HT16K33
<br>I2Cdevlib
<br>IRremoteESP8266
<br>LiquidCrystal_I2C
<br>MechInputs
<br>NewPingESP8266
<br>pubsubclient
<br>RN2483-Arduino-Library
<br>SDM_Energy_Meter
<br>SerialDevices
<br>SparkFun_APDS-9960_Sensor_Arduino_Library
<br>TinyGPSPlus-1.0.2

<h2>Requirements</h2>
Breakout-Board @I2C-address 0x76
Regular ESP8266-Board as nodeMCU or Wemos D1+D2 mini
Download of an actual ESPEasy-Version.
Copy original \src-Ffolder to \ESPEasy and use this Folder for an Arduino-compile.

<h2>Hints and errror fixing</h2>
*   overwrite eventually existing file from playgound with "Arduino: add file: _P119_BME680.ino"
*   overwrite ESPEasy\EspEasy.ino file with the Versio of this project.
*   in case of having trouble with Arduino public libraries:     
    Change default Arduino libraries-folder in i.e.  * *C:\Users\js\Documents\Arduino\libraries* * to i.e.  * *D:\Temp\ESPEasy-mega.org\lib* *. Exclusively to ESPEasy-Lib-Folder!  
* in order to achieve this (as admin):  **mklink /D libraries D:\Temp\ESPEasy-mega.org\lib** with command prompt in default Arduino-project-folder 
* SPIFFS settings -> from "none" to minimal "1M" 
* reset ESP two times.
