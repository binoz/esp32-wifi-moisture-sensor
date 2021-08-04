#include <WiFi.h>
#include "esp_wps.h"
#include <PubSubClient.h>


#define ESP_WPS_MODE      WPS_TYPE_PBC
#define ESP_MANUFACTURER  "ESPRESSIF"
#define ESP_MODEL_NUMBER  "ESP32"
#define ESP_MODEL_NAME    "ESPRESSIF IOT"
#define ESP_DEVICE_NAME   "ESP STATION"

const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic = "binoz/esp32/moisture";
const unsigned int mqtt_port = 1883;

const int LED_PIN = 14;
int blink_counter = 0;

static esp_wps_config_t config;

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

bool is_wifi_connected(){
  return WiFi.status() == WL_CONNECTED;
}

void log_wifi_event(WiFiEvent_t event){
  switch(event){
    
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("WiFi connected");
     
    break;
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("Got IP address: ");
    break;

     case SYSTEM_EVENT_STA_LOST_IP:
      Serial.println("Lost IP address: ");
    break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
       Serial.println("WiFi disconnected!");
       WiFi.reconnect();
    break;

    case SYSTEM_EVENT_STA_WPS_ER_SUCCESS: 
       Serial.println("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
    break;       
    
    case SYSTEM_EVENT_STA_WPS_ER_FAILED:        
      Serial.println("WPS Connection failed!");
    break;
    
    case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:   
      Serial.println("WPS Connection timeout!");
    break;
    
    
    default: break;
  }

}
void light_led(int duration){
  digitalWrite(LED_PIN,HIGH);
  delay(duration);
  digitalWrite(LED_PIN,LOW);
}

void led_blink(){
   
   blink_counter++;


   if (WiFi.status() != WL_CONNECTED){
     if (blink_counter % 2 == 0){
       digitalWrite(LED_PIN,HIGH);
     }else{
       digitalWrite(LED_PIN,LOW);
     }
     delay(1000);  
   }else{
     digitalWrite(LED_PIN,LOW);
   }
   
}

void on_wps_connected(WiFiEvent_t event){
    Serial.println("Connecting to WiFi");
    wps_stop();
    delay(10);
    WiFi.begin();
}

void on_wifi_connected(WiFiEvent_t event){
  
  WiFiClient wifi_client;
  PubSubClient mqtt_client(wifi_client);
  
  mqtt_client.setServer(mqtt_server, mqtt_port);  
  mqtt_client.connect("ESP32MoistureSensor");
  
  Serial.println("Sending mqtt message");

  int moisture = analogRead(39);
  char moisture_str[10]; 
  String(moisture).toCharArray(moisture_str,10);
  Serial.println(moisture);
  Serial.println(moisture_str);
  mqtt_client.publish(mqtt_topic,moisture_str);
  light_led(2000);

  Serial.println("Entering Sleep");
  esp_deep_sleep_start();
  Serial.println("Exiting Sleep");

}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN,OUTPUT);

    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    
    WiFi.mode(WIFI_MODE_STA);
    WiFi.onEvent(log_wifi_event);
    WiFi.onEvent(on_wps_connected,SYSTEM_EVENT_STA_WPS_ER_SUCCESS);
    WiFi.onEvent(on_wifi_connected,SYSTEM_EVENT_STA_GOT_IP);

    if (!is_wifi_connected()){
      
      Serial.println("WiFi is not connected.");
      
      if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER){
      
        Serial.println("Waking up from sleep, reconnetting wo WiFi using previous configuration");
        WiFi.begin();
      
      }else{
        Serial.println("Starting WPS");
        wps_init_config();
        wps_start();
      
      }
    
    }    
      
    esp_sleep_enable_timer_wakeup(5000000);
    

    
}



void loop(){
  led_blink();

}

