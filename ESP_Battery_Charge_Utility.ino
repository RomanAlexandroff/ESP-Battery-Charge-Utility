/* ********************************************************************************************** */
/*                                                                                                */
/*   ESP Battery Charge Utility                                        :::::::::        :::       */
/*   ESP_Battery_Charge_Utility.ino                                   :+:    :+:     :+: :+:      */
/*                                                                   +:+    +:+    +:+   +:+      */
/*   By: Roman Alexandrov <r.aleksandroff@gmail.com>                +#++:++#:    +#++:++#++:      */
/*                                                                 +#+    +#+   +#+     +#+       */
/*   Created: 2022/12/27 18:49:16                                 #+#    #+#   #+#     #+#        */
/*   Updated: 2023/10/16 14:21:41                                ###    ###   ###     ###         */
/*                                                                                                */
/*                                                                                                */
/*   This sketch will help you corelate ESP ADC readings of battery voltage with battery charge   */
/*   level in %.                                                                                  */
/*                                                                                                */
/* ********************************************************************************************** *

 
   !!! WARNING! DO NOT MEASURE BATTERIES RATED ABOVE 3.7V! IT WILL DAMAGE YOU ESP !!!
   !!! ESP measures VCC level internally. Do not connect ADC pin to anything and leave it floating !!!


  Instructions:
   1.  Fully charge your battery,
   2.  Meanwhile upload this sketch to your ESP,
   3.  After it is done, connect TX pin of the ESP to your computer via UART-TTL adapter or ESP-programator, DO NOT use 
       the USB port if your module has one – it will completely mess up the readings!
   4.  Open Serial monitor, set baud rate to 115200, enable automatic scrolling and time stamps,
   5.  Connect your fully charged battery to ESP (VCC and GND pins). Now Serial monitor should start showing readings.
       If you cannot see any entries in your Serial monitor, you need to connect ESP GND pin with GND of your 
       UART-TTL adapter or ESP-programator and restart Serial monitor. GND of the battery shall remain attached as-is.
   6.  After readings start to apear in Serial monitor, wait for 1 minute,
   8.  Disable automatic scrolling and find the lowest reading within the first 1 minute (see timestamps) — this number 
       equals to the battery absolute 100% charge. Your own sketch should take every higher reading as absolute 100% charge
   9.  Enable automatic scrolling again and let the battery drain out. Serial monitor will stop showing new readings 
       when your ESP browns out.
   10. Scroll up at least 1 minute from the last reading (see timestamps), 
   11. Find the highest reading within the 2nd minute from the end — this number equals to the battery absolute 0% charge, 
       Your own sketch should take every lower reading as absolute 0% charge             
 
  Tips:
   - Charging the battery up to its absolute 100% and draining it out to its absolute 0% may negatively influence the battery 
     health over time and result in its shorter lifespan. To prevent this from happening consider creating a separate 
     charge level scale for the user to see. Let's say, user 100% charge may be equal to absolute 90% charge and 
     user 0% charge == absolute 10% charge. Having said that, you should remember that even though ESP commonly browns out 
     at 2,7V (this equals to the measured absolute 0% charge), a common battery can be safely drained lower — to 2,5V. 
     In other words
         4,2V == Battery MAX == ESP MAX == absolute 100% charge
         3,2V ==                ESP MIN == absolute 0% charge         (below this level Wi-Fi function of ESP stops working) 
         2,8V ==                ESP OFF                               (below this level all ESP functions stop working)
         2,5V == Battery MIN                                          (BY ALL MEANS DO NOT ALLOW DISCHARGE BELOW THIS LEVEL!)
   - Do you have other modules that will work side by side with ESP connected to the same battery? Check the minumum level
     of voltage that they need! E.g. Ai-Thinker A9 GSM module requires minumum 3,3V for it to operate. If the device design
     does not include voltage booster to keep A9 GSM module running even on lower voltages, you have to take 3,3V as your
     absolute 0% charge instead of 3,2V!
     
 
   Initial formula to convert ADC measurements into Battery charge percentage %:  
     Battery charge = (ADC_Reading - Battery_Min_Reading) / ((Battery_Max_Reading - Battery_Min_Reading) / 100).
     where Battery_Max_Reading is the number from Serial monitor that you took for absolute 100% charge; 
     Battery_Min_Reading is the number from Serial monitor that you took for absolute 0% charge;
     ADC_Reading are the readings that ESP makes with "ESP.getVcc()" command while running your program.
   
   Full code to convert ADC measurements into Battery charge percentage %. Use this code in your sketch:
     #include <math.h>              // add this library for the "ceil()" command
     int battery;                   // put it as a global variable (above void setup()) so you can use it everywhere in your code
     battery = ceil((ESP.getVcc() - Battery_Min_Reading) / Battery_Coefficient);     // the "ceil()" command rounds up the result
     if (battery <= 0) battery = 0;
     if (battery >= 100) battery = 100;
     // Calculate Battery_Coefficient == (Battery_Max_Reading - Battery_Min_Reading) / 100
 
* *********************************************************************************************** */

#include <string.h>
#ifdef ESP32
  #include <driver/adc.h>
  #include "esp_adc_cal.h"
  #include "WiFi.h"
#else //ESP8266
  #include "ESP8266WiFi.h"
  ADC_MODE(ADC_VCC);
#endif

#define LOAD         //uncomment the line to turn on the Load – it will drain the battery faster to give you the results quicker

uint32_t  battery;
int       networks;

void  setup(void)
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(500);
    #ifdef ESP32
        adc1_config_width(ADC_WIDTH_12Bit);
        adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_11db);
    #endif
}

void  loop(void)
{
    #ifdef ESP32
        battery = adc1_get_raw(ADC1_CHANNEL_0);
    #else //ESP8266
        battery = ESP.getVcc();
    #endif
    #ifdef LOAD
        networks = WiFi.scanNetworks();
        Serial.printf("number of discovered networks: %d\n", networks);
        WiFi.scanDelete();
    #endif
    Serial.println(battery);
    delay(1000);  
}

 
