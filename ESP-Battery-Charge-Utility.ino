/* ********************************************************************************************** */
/*                                                                                                */
/*   ESP Battery Charge Utility                                        :::::::::        :::       */
/*   ESP-Battery-Charge-Utility.ino                                   :+:    :+:     :+: :+:      */
/*                                                                   +:+    +:+    +:+   +:+      */
/*   By: Roman Alexandrov <r.aleksandroff@gmail.com>                +#++:++#:    +#++:++#++:      */
/*                                                                 +#+    +#+   +#+     +#+       */
/*   Created: 2022/12/27 18:49:16                                 #+#    #+#   #+#     #+#        */
/*   Updated: 2023/10/16 14:21:41                                ###    ###   ###     ###         */
/*                                                                                                */
/*                                                                                                */
/*   This utility will help you corelate your ESP ADC readings of battery voltage with battery    */
/*   charge level in %.                                                                           */
/*                                                                                                */
/* ********************************************************************************************** *

   !!! WARNING! DO NOT MEASURE BATTERIES RATED ABOVE 3.7V! IT WILL DAMAGE YOU ESP !!!
   !!! ESPs measure VCC level internally. Do not connect ADC pin to anything and leave it floating !!!


  INSTRUCTIONS:
   1.  Fully charge your battery,
   2.  Meanwhile upload this sketch to your ESP,
   3.  After it is done and the battery is fully charged, connect TX pin of the ESP to your computer via UART-TTL adapter 
       or ESP-programator, DO NOT use the USB port if your module has one – it will mess up the readings!
   4.  Connect your fully charged battery to the ESP (VCC and GND pins). Connect the GND of the battery to the GND of your
       UART-TTL adapter or ESP-programator. The ESP, the battery and the adapter have to have common GND for the Serial
       monitor to work properly.
   5.  Open the Serial monitor, set baud rate to 115200, the Serial monitor should have started showing readings.
   6.  After the readings start to apear in the Serial monitor, pick top 4 readings and calculate their average — this 
       number equals to the battery absolute 100% charge. In this file it is called Battery_Max_Reading.
   7.  Enable automatic scrolling and let the battery drain out. The Serial monitor will stop showing new readings 
       when your ESP browns out,
   8.  You might need to disable automatic scrolling and scroll up to find the last readings, 
   9.  Pick 4 bottom readings and calculate their average — this number equals to the battery absolute 0% charge. In this 
       file it is called Battery_Min_Reading,
   12. Calculate Battery_Coefficient using the following formula:
            Battery_Coefficient == (Battery_Max_Reading - Battery_Min_Reading) / 100
   13. Paste the obtained constants values into the battery measuring sketch example below,
   14. Copy the sketch example and adapt it for your project.          

       Sketch to measure battery level in % in your project:
       
    --------------------------------------------------------------------------------------------------------

    #include <math.h>                                          // add this library for the "ceil()" function
    #ifdef ESP32
        #include <driver/adc.h>
    #else //ESP8266
        ADC_MODE(ADC_VCC);
    #endif

    short  ft_battery_check(void)
    {
        short i;
        short battery;

        i = 4;
        while (i)
        {
            #ifdef ESP32
                battery += ceil((adc1_get_raw(ADC1_CHANNEL_0) - Battery_Min_Reading) / Battery_Coefficient);
            #else //ESP8266
                battery += ceil((ESP.getVcc() - Battery_Min_Reading) / Battery_Coefficient);
            #endif
            i--;
        }
        battery = battery / 4;                                              // counting average of 4 samples
        if (battery < 0)
            battery = 0;
        if (battery > 100)
            battery = 100;
        return (battery);
    }
    
    --------------------------------------------------------------------------------------------------------  
     
 
  NOTES:  
   - Charging the battery up to its absolute 100% and draining it out to its absolute 0% may negatively influence the battery 
     health over time and result in its shorter lifespan. To prevent this from happening consider introducing a separate 
     "virtual" charge level scale for the user to see. Let's say, user 100% charge may be equal to absolute 90% charge and 
     user 0% charge == absolute 10% charge. 
   - ESP commonly browns out later than its Wi-Fi modem stops working. It means that even when it is unable to connect
     to Wi-Fi it still can make calculations, measurements, outputs and save information. Here is a table of what happens 
     while the battery level goes down:

         4,2V == Battery MAX == ESP MAX == absolute 100% charge       (max charge for the 3.7V-rated batteries)
         3,2V ==                ESP MIN == absolute 0% charge         (below this level Wi-Fi functions of ESP stop working) 
         2,8V ==                ESP OFF                               (at this level ESP browns-out and stops working)
         2,5V == Battery MIN                                          (by all means do not allow discharge below this level!)
         
   - Do you have other modules that will work side by side with the ESP connected to the same battery? Check the minumum level
     of voltage that they need! E.g. Ai-Thinker A9 GSM module requires minumum 3,3V for it to operate. If the device design
     does not include voltage booster to keep A9 GSM module running even on lower voltages, you have to take 3,3V as your
     absolute 0% charge instead of 3,2V.
   - The sketch above uses a simplified version of the formula to convert ADC measurements into Battery charge percentage %,
     which is the following:  
            Battery Charge == (ADC_Reading - Battery_Min_Reading) / ((Battery_Max_Reading - Battery_Min_Reading) / 100)
     and the "ceil()" function, which rounds up the result of the calculation.
 
* *********************************************************************************************** */


#include <string.h>
#ifdef ESP32
  #include "WiFi.h"
  #include <driver/adc.h>
#else //ESP8266
  #include "ESP8266WiFi.h"
  ADC_MODE(ADC_VCC);
#endif

//#define LOAD         //uncomment the line to turn on the Load – it will drain the battery faster to give you the results quicker

uint32_t  battery;
int       networks;
uint32_t  for_this_long = 1000;

void  setup(void)
{
    Serial.begin(115200);
    #ifdef ESP32
        adc1_config_width(ADC_WIDTH_12Bit);
        adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_11db);
    #endif
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(500);
}

void  loop(void)
{
    #ifdef LOAD
        networks = WiFi.scanNetworks();
        Serial.printf("number of discovered networks: %d\n", networks);
        WiFi.scanDelete();
        for_this_long = 100;
    #endif
    delay(for_this_long);
    #ifdef ESP32
        battery = adc1_get_raw(ADC1_CHANNEL_0);
    #else //ESP8266
        battery = ESP.getVcc();
    #endif
    Serial.println(battery);  
}

 
