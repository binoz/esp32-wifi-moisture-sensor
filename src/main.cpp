#include <WiFi.h>
#include "esp_wps.h"


#define ESP_WPS_MODE      WPS_TYPE_PBC
#define ESP_MANUFACTURER  "ESPRESSIF"
#define ESP_MODEL_NUMBER  "ESP32"
#define ESP_MODEL_NAME    "ESPRESSIF IOT"
#define ESP_DEVICE_NAME   "ESP STATION"

static esp_wps_config_t config;
RTC_DATA_ATTR bool wps_success = false;


void wps_init_config(){
  config.wps_type = ESP_WPS_MODE;
  config.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
  strcpy(config.factory_info.manufacturer, ESP_MANUFACTURER);
  strcpy(config.factory_info.model_number, ESP_MODEL_NUMBER);
  strcpy(config.factory_info.model_name, ESP_MODEL_NAME);
  strcpy(config.factory_info.device_name, ESP_DEVICE_NAME);
}

void wps_start(){
    if ( esp_wifi_wps_enable(&config)){
      Serial.println("WPS Enable Failed");
    } else if(esp_wifi_wps_start(0)){
      Serial.println("WPS Start Failed");
    }
}

void wps_stop(){
    if(esp_wifi_wps_disable()){
      Serial.println("WPS Disable Failed");
    }
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : 
      Serial.println("Wakeup caused by timer"); 
    break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

bool is_wifi_connected(){
  return WiFi.status() == WL_CONNECTED;
}

void handle_wifi_event(WiFiEvent_t event){
  switch(event){
    
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("WiFi connected");
     
    break;

    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("Got IP address: ");
      Serial.println(WiFi.localIP());
    break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
       Serial.println("WiFi disconnected!");
    break;

    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS: /**< ESP32 station wps succeeds in enrollee mode */
      Serial.println("WPS Connection successfull");
      wps_success = true;
      wps_stop();
      delay(10);
      WiFi.begin();
      Serial.println(WiFi.status());
    break;       
    
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:        /**< ESP32 station wps fails in enrollee mode */
      Serial.println("WPS Connection failed!");
    break;
    
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:       /**< ESP32 station wps timeout in enrollee mode */
      Serial.println("WPS Connection timeout!");
    break;
    
    
    default: break;
  }

}

void setup_wifi(){
  WiFi.mode(WIFI_MODE_STA);
  WiFi.onEvent(handle_wifi_event);

  if (!is_wifi_connected()){
    Serial.println("WiFi is not connected.");
    if (!wps_success){
       wps_init_config();
       wps_start();
    }else{
      Serial.println("Reconnecting");
      WiFi.reconnect();
    }
   
  }
}


void setup_sleep(){
  esp_sleep_enable_timer_wakeup(5000000);
}



void loop(){
 
}

void setup()
{
    Serial.begin(115200);

    //print_wakeup_reason();
    
    setup_wifi();
    setup_sleep();
    
    
    while (!is_wifi_connected()){
       Serial.println("waiting for wifi to connect...");
       delay(1000);

    }

    Serial.println("Entering Sleep");
    esp_deep_sleep_start();
    Serial.println("Exiting Sleep");
    
}

