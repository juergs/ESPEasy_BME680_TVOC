/* ----------------------------------
    File: js_bme680.c
    author: juergs @20190906
----------------------------------*/
#include <SimpleKalmanFilter.h>
#include <math.h>
#include <js_bme680.h>

   // #define BME680_DEFAULT_ADDRESS  (0x77)     ///< The default I2C address
   // #define BME680_SECONDARY_ADDRESS (0x76) 

    uint8_t         _I2C_BME680_ADDRESS   = BME680_I2C_ADDR_PRIMARY;  // 0x76 should be choosen as default address 

    SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.01);

    float           tVoc = 0; 
    float           resFiltered;                          // low pass
    float           base;
    
    unsigned long   prevBme680Millis    = millis();             // counter main loop for BME 680
    unsigned long   intervalBme680      = 10000;                // 10 sec update interval default
    bool            bme680VocValid      = false;                // true if filter is initialized, ramp-up after start
    char            bme680Msg[128];                             // payload
    bool            _isValidIaq         = false;                // true if baseline is stable
    
    struct 
    { 
        float         t_offset            = -.5;                 // offset temperature sensor
        float         h_offset            = 1.5;                 // offset humitidy sensor  
        uint32_t      signature           = 0x49415143;          // 'IAQC'
    } preload, param;                                       //--- stable new baseline counter (avoid short-term noise)    

    typedef struct 
    {       
        float   fTemp         = 0.0;        
        float   fHum          = 0.0;        
        float   fAbsHum       = 0.0;        
        float   fPress        = 0.0;        
        float   fTvoc         = 0.0;
        float   fTvocEst      = 0.0;                // Kalman filtered value 
        float   fDew          = 0.0;        
        float   fGas          = 0.0;        
        float   fAlt          = 0.0;    
        float   fAltCalib     = 0.0;
        float   fSeaLevel     = 0.0; 
    } GDATA_TYP; 
    
    GDATA_TYP gdata;   

//----------------------------------------------------------------------
String JS_BME680Class::getFloatValueAsString(float value) 
{    
    char str_temp[6];
    dtostrf(value, 4, 2, str_temp);
    return str_temp;  
}
//----------------------------------------------------------------------
void JS_BME680Class::set_bme680_offset_temp(float toffset) 
{  
    param.t_offset = toffset;   
}
//----------------------------------------------------------------------
void JS_BME680Class::set_bme680_offset_hum(float hoffset) 
{
    param.h_offset = hoffset;   
}
//----------------------------------------------------------------------
void JS_BME680Class::set_bme680_device_address(uint8_t addr) 
{
    _I2C_BME680_ADDRESS = addr; 
}
//----------------------------------------------------------------------
JS_BME680Class::JS_BME680Class()
{
    //do_begin(); 
}
//----------------------------------------------------------------------
bool   JS_BME680Class::isIAQValid()
{
    return(_isValidIaq); 
}
//----------------------------------------------------------------------    
float JS_BME680Class::getTemp(void)
{
    return (gdata.fTemp) ; 
}
//----------------------------------------------------------------------      
float JS_BME680Class::getPress(void)
{
    return (gdata.fPress) ; 
}
//----------------------------------------------------------------------      
float JS_BME680Class::getHum(void)
{
    return (gdata.fHum) ; 
}
//----------------------------------------------------------------------
float JS_BME680Class::getAlt(void)
{
    //return (gdata.fAlt) ; 
    return (1.0 - pow((float)bme680.pressure / 100.0f / BME680_SEALEVEL, 0.190284)) * 287.15 / 0.0065;
}
//----------------------------------------------------------------------
float JS_BME680Class::getCalibAlt()
{
    //return (gdata.fAltCalib) ; 
    return (1.0 - pow((float) bme680.pressure / BME680_SEALEVEL, 0.190284)) * 287.15 / 0.0065;
}
//----------------------------------------------------------------------
float JS_BME680Class::getGasRes(void)
{
    return (gdata.fGas) ;
}
//----------------------------------------------------------------------
float JS_BME680Class::getSeaLevel()
{
    return ((float) BME680_SEALEVEL) ; 
}
//----------------------------------------------------------------------
// float JS_BME680Class::readSeaLevel(float altitude)
// {
//   return (bme680_data.pressure / pow(1.0 - (altitude / 44330.0), 5.255));
// }
//----------------------------------------------------------------------
float JS_BME680Class::getTVoc(void)
{
    if (useFilteredTvocOutput)
    {
       return (gdata.fTvocEst) ; 
        if(useArduinoDebugOutput)
         Serial.println(F("\tJS_BME680  retuns gdata.fTvocEst" ));          
    }
    else
    {
       return (gdata.fTvoc) ; 
       if(useArduinoDebugOutput)
          Serial.println(F("\tJS_BME680  retuns gdata.fTvoc" ));            
    }       
}
//----------------------------------------------------------------------
float JS_BME680Class::getTVocFiltered(void)
{
    return (gdata.fTvocEst) ; 
}
//----------------------------------------------------------------------
unsigned long JS_BME680Class::get_bme680Interval()
{
    return (intervalBme680); 
}
//----------------------------------------------------------------------
float JS_BME680Class::absHum(float temp, float hum)
{
    double sdd, dd;
    sdd=6.1078 * pow(10,(7.5*temp)/(237.3+temp));
    dd=hum/100.0*sdd;
    return (float)216.687*dd/(273.15+temp);
}
//----------------------------------------------------------------------
float JS_BME680Class::dewPoint(float temp, float hum)
{
    double A = (hum/100) * pow(10, (7.5*temp / (237+temp)));
    return (float) 237*log10(A)/(7.5-log10(A));
}
//----------------------------------------------------------------------
int64_t JS_BME680Class::get_timestamp_us()
{
    return (int64_t)millis() * 1000;
}
//----------------------------------------------------------------------
int64_t JS_BME680Class::serial_timestamp()
{
    return (get_timestamp_us() / 1e6); 
}
//----------------------------------------------------------------------
void JS_BME680Class::do_bme680_measurement()
{
    getBme680Readings() ;  //--- with handling of prevBme680Millis
}
//----------------------------------------------------------------------
void JS_BME680Class::getBme680Readings() 
{
    prevBme680Millis  = millis(); // counter main loop for BME 680
    
    if (! bme680.performReading()) 
    { 
      #ifdef BME680_DEBUG
       Serial.println("\tbme680.performReading not ready yet!" );        
      #endif     
      return;
    };

    //--- first stage measurements
    float     t = bme680.temperature + param.t_offset;    
    float     h = bme680.humidity + param.h_offset;    
    float     a = absHum(t, h);
    float     d = dewPoint(t, h);    
    float     p = bme680.pressure / 100.0F;    
    uint32_t  r = bme680.gas_resistance; // raw R VOC

	gdata.fTemp   = t; 
	gdata.fHum    = h; 
	gdata.fPress  = p;       
	gdata.fGas    = r;  
	gdata.fAbsHum = a; 

    if (!bme680VocValid )
    {
      _isValidIaq = false; 
      gdata.fTvoc   = 0.0 ; 
      gdata.fTvocEst = 0.0; 
    }
	
    if (r == 0) 
    {
      if (useArduinoDebugOutput)
      {
        Serial.println(F("\tJS_BME680.resitance beeing zero! First Reading?" ));        
      }

      return;      //--- first reading !=0 accepted, 0 is invalid   
    }

    //--------------------------------------------------------------------
    //--- from here we read  r_gas values, but haven't calculated tVoc yet 
    //--------------------------------------------------------------------

    if (!bme680VocValid )
    { 
        //--- allow 300 sec (5 min) to warm-up sensor for stable voc resistance (300000ms)
        if ( (millis() > 300000) ) 
        {       
            bme680VocValid = true;           
        }else{
            if (useArduinoDebugOutput)
            {
                Serial.print(F("\tJS_BME680.bme680VocValid not ready! Wait about 300 sec (5 min) to warm up ... " ));
                Serial.println( ( (300000 - millis() ) / 1e3), 0 );
            }
            return;
        }
    }
        
    //------------------------------
    //--- up from here we read tVoc 
    //------------------------------

	resFiltered += 0.3 * ((r * a) - resFiltered);
	base += 0.0001 * (resFiltered - base);
	// base line correction, low-pass cause of sensor latency 
	if (base < resFiltered && !_isValidIaq) {
		base += 0.5 * (resFiltered - base);
	} else if (base < resFiltered && _isValidIaq) {
		base += 0.005 * (resFiltered - base);
	} else if (!_isValidIaq) {
		_isValidIaq = true;
	};
	float ratio = base / resFiltered;
	if (ratio < 1) {
		ratio = 1;
	};
	tVoc = 125 + ((ratio - 1) * 1500);
    gdata.fTvoc = tVoc; 

    float tvoc_estimated_value = simpleKalmanFilter.updateEstimate(tVoc);
    gdata.fTvocEst = tvoc_estimated_value;
  
    if (useArduinoDebugOutput)
    {
      Serial.println(F("\t--------------------------------------"));        
      Serial.print(F("\t Timestamp: \t\t")); Serial.println( (int64_t) get_timestamp_us() / 1e6);
      Serial.print(F("\t tVoc:\t\t\t")); Serial.println(tVoc);
      Serial.print(F("\t tVocEst:\t\t")); Serial.println(tvoc_estimated_value);
      Serial.print(F("\t Ratio:\t\t\t")); Serial.println(ratio);
      Serial.print(F("\t Ratio_log:\t\t"));Serial.println(log(ratio));
      Serial.print(F("\t ABC_base:\t\t")); Serial.println(base);
      Serial.print(F("\t Abs_Hum:\t\t")); Serial.println(a);
      Serial.print(F("\t Resistance (raw):\t")); Serial.println(r);
      Serial.print(F("\t Resistance (filt.):\t")); Serial.println(resFiltered);
      Serial.print(F("\t Temp:\t\t\t")); Serial.println(t);
      Serial.print(F("\t Hum:\t\t\t")); Serial.println(h);
      Serial.print(F("\t Press:\t\t\t")); Serial.println(p);
      Serial.print(F("\t Dewpoint:\t\t")); Serial.println(d);
      Serial.print(F("\t Alt:\t\t\t")); Serial.println(getAlt());
      Serial.print(F("\t T-Offs:\t\t\t")); Serial.println(param.t_offset);
      Serial.print(F("\t H-Offs:\t\t\t")); Serial.println(param.h_offset);
    }

    //--- use the Arduino-Serial Ploter for a good visualization
    if (useArduinoPlotOutput)
    {
      Serial.print(tVoc);
      Serial.print(",");
      Serial.print(tvoc_estimated_value,0);
      Serial.print(",");
      Serial.print(t,0);
      Serial.print(",");
      Serial.print(h,0);
      Serial.println();
    }    

  };  
//----------------------------------------------------------------
void JS_BME680Class::do_begin()
{    
    delay(1000);   // wait debug console to settle 

    DEBUG_OUT("\n*** JS_BME680Class started!\n");

    //--- set I2C-Clock
//    #ifdef ESP8266
      //Wire.setClockStretchLimit(1000); //--- Default is 230us, see line78 of https://github.com/esp8266/Arduino/blob/master/cores/esp8266/core_esp8266_si2c.c
      //Wire.setClock(400000);
      //--- (SDA,SCL) D1, D2 enable I2C for Wemos D1 mini SDA:D2 / SCL:D1 
      //Wire.begin(D2, D1);
      //DEBUG_PRINT(F("Enabled: I2C for Wemos D1 mini SDA:D2 / SCL:D1 "));
//    #else
      //Wire.begin();
//    #endif 

   if (!bme680.begin(_I2C_BME680_ADDRESS)) 
    {
        delay(3000);
        while (1);
    }     
    else
    {
        //DEBUG_PRINT(F("I2C: ok BME680 sensor found! :-) "));
    }
    
    //--- set up oversampling and filter initialization
    bme680.setTemperatureOversampling(BME680_OS_8X);
    bme680.setHumidityOversampling(BME680_OS_2X);
    bme680.setPressureOversampling(BME680_OS_4X);
    bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme680.setGasHeater(320, 150);    // 320*C for 150 ms
}

//---------------------------------
//---  declare the static instance 
JS_BME680Class JS_BME680;
