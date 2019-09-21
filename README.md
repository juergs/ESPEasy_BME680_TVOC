# ESPEasy_BME680_TVOC
<h2>ESPEasy_BME680_TVOC without need of proprietary BSEC static lib.</h2>
<h4>Special adapted ESPEasy-Version for BME680-Measurement of TVOC in [ppm].<br>
Using a modified <code><b><em>_P119_BME680.ino</em></b></code>-Version as replacement. (source: Playground) <br>
Added UDP SLINK send feature.
</h4>


<h2>Objective</h2>
Available BME680-Libraies use only Gas-Resistance as output of IAQ. 
This is not usable. Therefore  we use our own calculation of tVoc with output in ppm units.
Many thanks to Jörg, having done the basic work. 
<br>See him @ https://github.com/herrmannj .
<br>Initial work is done within the FHEM (german home automation forum) in 
<br>https://forum.fhem.de/index.php/topic,78619.msg972248.html#msg972248
<br>or  here:
<br>https://forum.fhem.de/index.php?topic=96241.285

<h2>Hardware</h2>
Use I2C Interface D1 and D2 of ESP8266. Set SD0 to Low for 0x76 address.

ESP12E ----------- BME680 @ 0x76 (SDO = GND)
<br>GND ------------> GND
<br>3.3V ------------> 3.3V
<br>D2 ------------> SDA
<br>D1 ------------> SCL

<h2>Libraries</h2>
These additional bold libraries are needed for using BME680-Sensor in ESPEasy \lib-folder.:

**Adafruit_BME680_Library**
<br>**Adafruit_Unified_Sensor**
<br>**js_BME680_prod**
<br>**Adafruit_Sensor**

<u>These were part of the regular distribution:</u>
<small><em>
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
    </em></small>
<h2>Requirements</h2>
Breakout-Board @I2C-address 0x76
Regular ESP8266-Board as nodeMCU or Wemos D1+D2 mini
Download of an actual ESPEasy-Version.
Copy original \src-Ffolder to \ESPEasy and use this Folder for an Arduino-compile.

<h2>Hints and errror fixing</h2>
*   the file _P119_BME680.ino is chosen for replace playground version for bm680.
*   overwrite eventually existing file from playgound with "Arduino: add file: _P119_BME680.ino" <br>
*   overwrite ESPEasy\EspEasy.ino file with the Version of this project.<br>
*   in case of having trouble with Arduino public libraries:     <br>
    &rArr; Change default Arduino libraries-folder in i.e. <i>C:\Users\js\Documents\Arduino\libraries</i> to i.e.  <i>D:\Temp\ESPEasy-mega.org\lib</i>. (Setting Arduino-configuration of libraries exclusively to ESPEasy-Lib-Folder!)<br>
* in order to achieve this (as admin): <b>mklink /D libraries D:\Temp\ESPEasy-mega.org\lib</b> with command prompt in default Arduino-project-folder <br>
* SPIFFS settings -> from "none" to minimal "1M" <br>
* reset ESP two times.<br>
<h2>Pictures<h2>
<img src="https://cdn-shop.adafruit.com/970x728/3660-00.jpg" height="300"/>
<br>
<img src="https://github.com/juergs/ESPEasy_BME680_TVOC/blob/master/ESP_Easy_BME680_Konfiguration.png" height="300"/>
<br>
<img src="https://github.com/juergs/ESPEasy_BME680_TVOC/blob/master/BME680_Me%C3%9Fwerte_mit_Kalman_Filter_4.png" height="300"/>
<br>
<img src="https://github.com/juergs/ESPEasy_BME680_TVOC/blob/master/EspEasy_Devices.png" height="300"/>    
<br>
<img src="https://github.com/juergs/ESPEasy_BME680_TVOC/blob/master/BME680_IAQ_vs_TVOC.png" height="500"/>    
<br>
<img src="https://github.com/juergs/ESPEasy_BME680_TVOC/blob/master/3_mal_Slink_vs_1_mal_IAQpng.png" height="500"/>    
<br>
<br>
<img src="https://github.com/juergs/ESPEasy_BME680_TVOC/blob/master/bme680.gif" height="500"/>    
<br>

<h2>Upload-Tools</h2>
Are found in \dist-folder of EspEasy.

<h2> EspEasy </h2>
1. Download firmware as binary including flash tool <br>
↓<br>
2. Connect the ESP to Windows PC<br>
Using either USB/UART of board or separate USB/TTL adapter<br>
↓<br>
3. Write firmware using flash tool<br>
Note necessity for GPIO to be LOW to enter flashmode<br>
↓<br>
4. Restart ESP. WiFi AP "ESP_Easy_0" will appear, password: configesp<br>
(prior to 2.0 the AP was named ESP_0)<br>
If you're not automatically taken to the log-in page, browse to 192.168.4.1<br>
↓<br>
5. Search for you routers WiFi and connect<br>
(if you have multiple AP they will all show up with the same SSID name multiple times)<br>
↓<br>
6. Reconnect to your WiFi and enter IP adress shown on previous screen    <br>
    
<h2>Contribution</h2>
Every Contribution to this repository will be highly appreciated! 
Don't fear to create pull requests which enhance or fix the library as ultimatly you are going to help everybody.
