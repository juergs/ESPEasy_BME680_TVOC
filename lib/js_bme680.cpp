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

  // bool            useFilteredTVocOutput = false;  
  // bool            useArduinoPlotOutput  = false;
  // bool            useArduinoDebugOutput = true; 

    float           tVoc = 0; 
    float           resFiltered;                          // low pass
    float           aF = 0;
    
    unsigned long   prevBme680Millis    = millis();             // counter main loop for BME 680
    unsigned long   intervalBme680      = 10000;                // 10 sec update interval default
    bool            bme680VocValid      = false;                // true if filter is initialized, ramp-up after start
    char            bme680Msg[128];                             // payload
    bool            _isValidIaq         = false;                // redundance to bme680VocValid?
    
    //--- automatic baseline correction
    uint32_t        bme680_baseC        = 0;                    // highest adjusted r (lowest voc) in current time period
    float           bme680_baseH        = 0;                    // abs hum (g/m3)
    unsigned long   prevBme680AbcMillis = 0;                      // ts of last save to nv 
    unsigned long   intervalBme680NV    = 604800000;                // 7 days of ms
    uint8_t         bDelay              = 0;  
    uint16_t        pressureSeaLevel    = 1013.25;            //default value of 1013.25 hPa  

    struct 
    { 
        float         t_offset            = -.5;                 // offset temperature sensor
        float         h_offset            = 1.5;                 // offset humitidy sensor  
        uint32_t      vocBaseR            = 0;                   // base value for VOC resistance clean air, abc 
        uint32_t      vocBaseC            = 0;                   // base value for VOC resistance clean air, abc  
        float         vocHum              = 0;                   // reserved, abc
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

// //----------------------------------------------------------------------  
// void JS_BME680Class::set_bme680_filtered_output(bool enable)
// {
//     useFilteredTVocOutput = enable; 
// }
// //----------------------------------------------------------------------
// void JS_BME680Class::enableDebugOutput(bool val) 
// {
//     useArduinoDebugOutput = val; 
// }
// //----------------------------------------------------------------------
// void JS_BME680Class::enablePlotOutput(bool state) 
// {
//     useArduinoPlotOutput = state; 
// }
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
    
    aF = (aF == 0 || a < aF)?a:aF + 0.2 * (a - aF);
    
    float     d = dewPoint(t, h);
    
    float     p = bme680.pressure / 100.0F;
    
    uint32_t  r = bme680.gas_resistance; // raw R VOC

    #define SEALEVELPRESSURE_HPA (1013.25)
    
    if (!bme680VocValid )
    {    
      //--- invalid tvoc reading
      _isValidIaq = false; 
      gdata.fTemp   = t; 
      gdata.fHum    = h; 
      gdata.fPress  = p;       
      gdata.fGas    = r;  
      gdata.fAbsHum = a; 
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

    uint32_t base = bme680Abc(r, a);       // update base resistance 

    if (!bme680VocValid && (millis() > 300000)) 
    {       
        //--- allow 300 sec (5 min) to warm-up sensor for stable voc resistance (300000ms)
        resFiltered = r;        // preload low pass filter
        bme680VocValid = true;           
    }
    else
    {
         _isValidIaq = false;
    }
    
    if (!bme680VocValid ) 
    {
      if (useArduinoDebugOutput)
      {
        Serial.print(F("\tJS_BME680.bme680VocValid not ready! Wait about 300 sec (5 min) to warm up ... " ));  Serial.println( ( (300000 - millis() ) / 1e3), 0 ); 
      }
      return;
    }
    
    //------------------------------
    //--- up from here we read tVoc 
    //------------------------------
    _isValidIaq = true;       

    resFiltered += 0.1 * (r - resFiltered);    

    float ratio = (float)base / (r * aF * 7.0F);

    float tV  = ( 1250 * log(ratio) ) + 125;                     // approximation    

    float tV2 = tVoc + 0.1 * (tV - tVoc);

    tVoc = (tVoc == 0 || isnan(tVoc) || isinf(tVoc) ) ? tV : tV2;       // global tVoc

    gdata.fTvoc = tVoc; 

    float tvoc_estimated_value = simpleKalmanFilter.updateEstimate(tVoc);    

    gdata.fTvocEst = tvoc_estimated_value; 
  
    if (useArduinoDebugOutput)
    {
      Serial.println(F("\t--------------------------------------"));        
      Serial.print(F("\t Timestamp: \t\t")); Serial.println( (int64_t) get_timestamp_us() / 1e6);     
      Serial.print(F("\t tVoc:\t\t\t")); Serial.println(tVoc); 
      Serial.print(F("\t tVocEst:\t\t")); Serial.println(tvoc_estimated_value); 
      Serial.print(F("\t Approx (tV):\t\t")); Serial.println(tV); 
      Serial.print(F("\t Approx (tV2):\t\t")); Serial.println(tV2);
      Serial.print(F("\t Ratio:\t\t\t")); Serial.println(ratio); 
      Serial.print(F("\t Ratio_log:\t\t"));Serial.println(log(ratio)); 
      Serial.print(F("\t ABC_base:\t\t")); Serial.println(base);             
      Serial.print(F("\t Abs_Hum:\t\t")); Serial.println(aF);      
      Serial.print(F("\t Resistance (raw):\t")); Serial.println(r); 
      Serial.print(F("\t Resistance (filt.):\t")); Serial.println(resFiltered);       
      Serial.print(F("\t Temp:\t\t\t")); Serial.println(t);
      Serial.print(F("\t Hum:\t\t\t")); Serial.println(a);
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
  //----------------------------------------------------------------------
  uint32_t JS_BME680Class::bme680Abc(uint32_t r, float a) 
  {   
    //--- automatic baseline correction
    uint32_t b = r * a * 7.0F;
    if (b > bme680_baseC && bDelay > 5) 
    {     
      //--- ensure that new baseC is stable for at least >5*10sec (clean air)
      bme680_baseC = b;
      bme680_baseH = a;
    } else if (b > bme680_baseC) 
    {
      bDelay++;
      //return b;
    } else 
    {
      bDelay = 0;
    };    
    return (param.vocBaseC > bme680_baseC)?param.vocBaseC:bme680_baseC;
  };
//----------------------------------------------------------------
void JS_BME680Class::do_begin()
{    
    delay(1000);   // wait debug console to settle 

    DEBUG_OUT("\n*** JS_BME680Class started!\n");

    //--- set I2C-Clock
    #ifdef ESP8266
      Wire.setClockStretchLimit(1000); //--- Default is 230us, see line78 of https://github.com/esp8266/Arduino/blob/master/cores/esp8266/core_esp8266_si2c.c
      Wire.setClock(400000);
      //--- (SDA,SCL) D1, D2 enable I2C for Wemos D1 mini SDA:D2 / SCL:D1 
      Wire.begin(D2, D1);
      //DEBUG_PRINT(F("Enabled: I2C for Wemos D1 mini SDA:D2 / SCL:D1 "));
    #else
      Wire.begin();
    #endif 

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
