#include "_Plugin_Helper.h"
#ifdef USES_P119
//#######################################################################################################
//#################### Plugin 120 BME680 I2C Temp/Hum/Barometric/Pressure/Gas Resistence Sensor  ########
//#######################################################################################################

/*******************************************************************************
 * Copyright 2017
 * Written by Rossen Tchobanski (rosko@rosko.net)
 * BSD license, all text above must be included in any redistribution
 *
 * Release notes:
   Adafruit_BME680 Library v1.0.5 required (https://github.com/adafruit/Adafruit_BME680/tree/1.0.5)
*****************************************************************************/


//#ifdef PLUGIN_BUILD_DEV
//#ifdef PLUGIN_BUILD_TESTING

#include <js_BME680.h>

//#ifndef PCONFIG
//  # define PCONFIG(n) (Settings.TaskDevicePluginConfig[event->TaskIndex][(n)])
//#endif // ifndef PCONFIG

#define PLUGIN_119
#define PLUGIN_ID_119         119
#define PLUGIN_NAME_119       "Environment - BME680 - TVOC "
#define PLUGIN_VALUENAME1_119 "Temperature"
#define PLUGIN_VALUENAME2_119 "Humidity"
#define PLUGIN_VALUENAME3_119 "Pressure"
#define PLUGIN_VALUENAME4_119 "TVOC"

#define SEALEVELPRESSURE_HPA (1013.25)

//JS_BME680 is declared in lib !

unsigned long 	cycleCounter  = 0;

unsigned long 	prevTime  = 0;

boolean Plugin_119_init = false;


// UDP - Multicast declarations
WiFiUDP udp; 
IPAddress ipMulti(239, 255, 255, 250);    // site-local
unsigned int portMulti = 2085;            // port
char packetIn[255];                       // UDP in-buffer
char packetOut[512];                      // UPD out-buffer

void sendUdpMessage(char* msg) 
{
  if (WiFi.status() == WL_CONNECTED && strlen(msg) != 0) {
    snprintf(packetOut, sizeof(packetOut), "T:IAQC;FW:1.0;ID:%06X;IP:%s;R:%d;%s", ESP.getChipId(), WiFi.localIP().toString().c_str(), WiFi.RSSI(), msg);
    DEBUG_PRINT(packetOut);
    udp.beginPacketMulticast( ipMulti, portMulti, WiFi.localIP() );
    udp.println(packetOut);
    udp.endPacket();
    strcpy(msg, "");
  };
};

boolean Plugin_119(byte function, struct EventStruct *event, String& string)
{
  boolean success = false;



  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_119;
        Device[deviceCount].Type = DEVICE_TYPE_I2C;
        Device[deviceCount].VType = Sensor_VType::SENSOR_TYPE_QUAD;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = false;
        Device[deviceCount].InverseLogicOption = false;
        Device[deviceCount].FormulaOption = true;
        Device[deviceCount].ValueCount = 4;
        Device[deviceCount].SendDataOption = true;
        Device[deviceCount].TimerOption = true;
        Device[deviceCount].GlobalSyncOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_119);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_119));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_119));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[2], PSTR(PLUGIN_VALUENAME3_119));
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[3], PSTR(PLUGIN_VALUENAME4_119));
        break;
      }
    //-----------------------------------------------
    // --- Init is performed during first read call !
    //-----------------------------------------------
    // case PLUGIN_INIT: 
    // {

    //     //--- start measuring 
    //     JS_BME680.do_begin(); 

    //     Plugin_119_init = true; 

    //     addLog(LOG_LEVEL_INFO, F("PLUGIN_INIT: JS_BME680 : ready + initialized!"));
    //     serial.println(F("PLUGIN_INIT: JS_BME680 : ready + initialized!")); 
  
    //     success = true;
    //     break;
         
    //     // } else {
    //     //clearPluginTaskData(event->TaskIndex);
    //   }
    //   break;
    // }

    // --- on load configuration dialog
    case PLUGIN_WEBFORM_LOAD:
      {        
        byte choice = Settings.TaskDevicePluginConfig[event->TaskIndex][0];
        int optionValues[2] = { 0x77, 0x76 };        
        addFormSelectorI2C(F("plugin_119_BME680_i2c"), 2, optionValues, choice);
        addFormNote(F("SDO Low=0x76, High=0x77"));
        
        addFormNumericBox(F("Altitude"), F("plugin_119_BME680_elev"), PCONFIG(1));
        addUnit(F("m"));

        //------------
        addFormSubHeader(F("Measurement options"));
        addFormCheckBox(F("Filter tVOC"), F("plugin_119_BME680_filtered_tvoc_enable"), PCONFIG(2) );

        addFormNumericBox(F("Temp-Offset"), F("plugin_119_BME680_toffs"), PCONFIG(3));
        addUnit(F("°C"));

        addFormNumericBox(F("Hum-Offset"), F("plugin_119_BME680_hoffs"), PCONFIG(4));
        addUnit(F("%"));

        //------------
        addFormSubHeader(F("Debug options"));
        addFormCheckBox(F("Plot output enable"), F("plugin_119_BME680_enable_plot_output"), PCONFIG(5) );
        addFormNote(F("Use with Arduino-Plotter on right COM-Port!"));

        addFormCheckBox(F("Debug output enable"), F("plugin_119_BME680_enable_debug_output"), PCONFIG(6) );
        addFormNote(F("Use Arduino-Serial-Monitor on right COM-Port!"));

        //------------
        addFormSubHeader(F("Communication options"));
        addFormCheckBox(F("UDP slink transmission enable"), F("plugin_119_BME680_enable_slink"), PCONFIG(7) );
        
        success = true;
        break;
      }

    // --- on submit on configuration dialog
    case PLUGIN_WEBFORM_SAVE:
      {
        //--- get int config values
        PCONFIG(0) = getFormItemInt(F("plugin_119_BME680_i2c"));        
        PCONFIG(1) = getFormItemInt(F("plugin_119_BME680_elev"));        
        PCONFIG(3) = getFormItemInt(F("plugin_119_BME680_toffs"));        
        PCONFIG(4) = getFormItemInt(F("plugin_119_BME680_hoffs"));

        //--- save checkboxes
        if (isFormItemChecked( F("plugin_119_BME680_filtered_tvoc_enable")) )
        {
          PCONFIG(2) = 1; 
        }
        else
        {
          PCONFIG(2) = 0;
        }
          
        //--- plot setting 
        if (isFormItemChecked( F("plugin_119_BME680_enable_plot_output")) )
        {
          PCONFIG(5) = 1;
        }
        else
        {         
          PCONFIG(5) = 0;
        } 
        //--- debug         
        if (isFormItemChecked( F("plugin_119_BME680_enable_debug_output")) )
        {
          PCONFIG(6) = 1;
        }
        else
        {
          PCONFIG(6) = 0;
        }


        if (isFormItemChecked( F("plugin_119_BME680_enable_slink")) )
        {
          PCONFIG(7) = 1;
        }
        else
        {
          PCONFIG(7) = 0;
        }
        
          
        success = true;
        break;
      }

    //--- on read interal set in configuration dialog
    case PLUGIN_READ:
      {
        //--- see if (Plugin_119_init) below 
        //--- here first start with init! Plugin_119_init = false
        if (!Plugin_119_init)
        {
            addLog(LOG_LEVEL_INFO, F("PLUGIN_READ - BME680  : init"));

            Plugin_119_init = true; 


            addLog(LOG_LEVEL_INFO, F("BME680  : PLUGIN_READ initialized."));

            String log = F("BME680  : PLUGIN_READ-Settings: I2C: ");
            log += PCONFIG(0) ;
            log += F(" Elevation: ");            
            log += PCONFIG(1);
            log += F(" Filter: ");            
            log += PCONFIG(2);
            log += F(" Toffs: ");            
            log += PCONFIG(3);
            log += F(" Hoffs: ");            
            log += PCONFIG(4);            
            log += F(" Plot: ");            
            log += PCONFIG(5);            
            log += F(" Debug: ");
            log += PCONFIG(6);            
            addLog(LOG_LEVEL_INFO, log);

            //-- checks 
            uint8_t use_this_i2c_adr = 0x76; 
            if (PCONFIG(0) != 0) 
            {
              //--- other than default: 0x77
              use_this_i2c_adr = (uint8_t) PCONFIG(0);             
            }
            bool use_Plotting = false; 
            
            //--- plot-swithed on?
            if (PCONFIG(5) != 0) 
            {
              //--- switch debugging on 
              use_Plotting = (uint8_t) PCONFIG(5);             
            }
            
            //--- debug-switched on?
            bool use_Debugging = false; 
            if (PCONFIG(6) != 0) 
            {
              //--- switch debugging on 
              use_Debugging = (uint8_t) PCONFIG(6);             
            }

            // --- Temp-offset 
            float t_offs = 0; 
            if (PCONFIG(3) != 0) 
            {
              t_offs = (float) PCONFIG(3);             
            }                           
            
            // --- HUM-offset 
            float h_offs = 0; 
            if (PCONFIG(4) != 0) 
            {
              t_offs = (float) PCONFIG(4);             
            }  
            
            // --- TVOCFiltered
            bool use_filtered_tvoc = false; 
            if (PCONFIG(2) != 0) 
            {
              //--- switch filtered output on 
              use_filtered_tvoc = true;             
            }  

            // --- setup code,  run once
			      JS_BME680.set_bme680_device_address(use_this_i2c_adr);  // may be ommitted in case of default-address 0x76 (SDO = GND), declare in case of address 0x77 (SDO = High)      
            JS_BME680.useArduinoDebugOutput    = use_Debugging; 
            JS_BME680.useArduinoPlotOutput     = use_Plotting;
            JS_BME680.useFilteredTvocOutput    = use_filtered_tvoc;
            JS_BME680.set_bme680_offset_hum ( h_offs); 
            JS_BME680.set_bme680_offset_temp (t_offs);
      
            //--- start measuring 
            JS_BME680.do_begin(); 

			      addLog(LOG_LEVEL_INFO, F("JS_BME680-Read : ready + not initialized code done! do_begin called"));
            
            bool debugEnabled = (uint8_t) PCONFIG(6);
            if (debugEnabled)
              Serial.println(F("PLUGIN_INIT: JS_BME680 : ready + initialized!"));
			
            success = true;
            break;
        }
        //-------------------------------------------------------------------------------------------------------
        //--- this code is done by having initialized BME680 on first read and not on instanciating class object! 
        //--- is established after 2nd Reading, after initialize with do_begin()-method. 
        //--- using above set parameters as as bme680 startup!
        //-------------------------------------------------------------------------------------------------------
        if (Plugin_119_init)
        {
              JS_BME680.do_bme680_measurement();  

              addLog(LOG_LEVEL_INFO, F("BME680-Read: performed measurement!"));              
              
              bool debugEnabled = (uint8_t) PCONFIG(6);
              if (debugEnabled) 
                Serial.println(F("PLUGIN_Read: JS_BME680 : init done, measure!"));

			
            // //--- put your main code here, to run repeatedly:
            // static unsigned long baseIntervall = JS_BME680.get_bme680Interval(); 
            // unsigned long currentMillis = millis();          
            //---bme680 data screen added
            // if (currentMillis - prevTime > baseIntervall) 
            // {   
              // --- ca. alle 10 Sekunden eine Messung aller Messgroessen      
              //JS_BME680.do_bme680_measurement();
            //   prevTime = currentMillis; 
            // }          


            //--- pass measurements to ESPEasy            
            UserVar[event->BaseVarIndex + 0] = JS_BME680.getTemp(); 
            UserVar[event->BaseVarIndex + 1] = JS_BME680.getHum();
            UserVar[event->BaseVarIndex + 2] = JS_BME680.getPress();
            UserVar[event->BaseVarIndex + 3] = JS_BME680.getTVoc();

            bool use_slink = false; 
            if (PCONFIG(7) != 0) 
            {
              //--- switch debugging on 
              use_slink = (uint8_t) PCONFIG(7);             
            }
            
            //--- send UDP-Message to fhem 
            char bme680Msg[128];   
            char str_temp[6];
            char str_hum[6];
            char str_absHum[6];
            char str_dewPoint[6];
            char str_pressure[16];
            //char str_altitude[8];
            char str_tVoc[8];
            char str_gas[8];
            char str_r[6];
            char str_filtered[6]; 
            char str_ratio[6]; 
            char str_base[6];
                        
            //--- using udp transmission (once every minute)
            // @fhem use these additional perl modules: https://github.com/herrmannj/AirQuality/tree/master/FHEM
            if (use_slink) 
            {
              dtostrf(JS_BME680.getTemp(), 4, 2, str_temp);
              dtostrf(JS_BME680.getHum(), 4, 2, str_hum);
              dtostrf(0, 4, 2, str_absHum);
              dtostrf(0, 4, 2, str_dewPoint);
              dtostrf(JS_BME680.getPress(), 3, 1, str_pressure);
              dtostrf(JS_BME680.getGasRes(), 3, 1, str_gas);
              dtostrf(JS_BME680.getTVoc(), 1, 0, str_tVoc);   
              dtostrf(0.0, 1, 0, str_r);      //--- unused
              dtostrf(0.0, 1, 0, str_filtered);  //--- unused   
              dtostrf(1.0, 1, 0, str_ratio);   //--- unused
              dtostrf(1.0, 1, 0, str_base);   //--- unused              
              
              //---  prepare to send UDP-message to fhem
              snprintf(bme680Msg
               , sizeof(bme680Msg)
               , "F:THPV;T:%s;H:%s;AH:%s;D:%s;P:%s;V:%s;R:%lu;DB:%lu;DF:%s;DR:%s;"
               , str_temp
               , str_hum
               , str_absHum
               , str_dewPoint
               , str_pressure
               , str_tVoc
               , str_r
               , str_base
               , str_filtered
               , str_ratio);
  
               sendUdpMessage(bme680Msg); 
  
               addLog(LOG_LEVEL_INFO, bme680Msg);  
            }
            
            //-- done
            success = true;
            break;
        }

        success = true;
        break;
      } //--- case pluginread

      case PLUGIN_ONCE_A_SECOND:
      {
        //code to be executed once a second. Tasks which do not require fast response can be added here
        // https://ae-bst.resource.bosch.com/media/_tech/media/application_notes/BST-BME680-AN014.pdf , page 10
        cycleCounter ++; 
        if (cycleCounter >= 12)
        {
          JS_BME680.do_bme680_measurement();
          cycleCounter = 0; 
        }
        success = true;

      }

  }
  return success;
}

#endif