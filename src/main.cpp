#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char *ssid = "Jwlan";
const char *password = "j0407650075";
const char *mqtt_server = "broker.mqtt-dashboard.com";
const char *mqtt_topic_interval = "prMzf8FpDKIFvcKJlPKh-G2PMBviTzcv2OkCVb4Ix";
const char *mqtt_topic_execute_length = "prMzf8FpDKIFvcKJlPKh-G2PMBviTzcv2OkCVb4Ix2";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
long int value = 0;
const int relayPin = D1;
const int ledPin = D2;

long action_interval_previousMillis = 0;
long action_execute_previousMillis = 0;
unsigned long int action_interval = 0;
unsigned long int action_length = 5000;
bool action_is_executing = false;

void setup_wifi(){
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect(){
  while (!client.connected()){
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())){
      Serial.println("connected!");
      client.subscribe(mqtt_topic_interval);
      client.subscribe(mqtt_topic_execute_length);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void callback(char *topic, byte *payload, signed int length){

  Serial.println("Message arrived: ");
  Serial.println(topic);

  bool input_valid = true;
  char buffer[length];
  for (int i = 0; i < length; i++){
    if(isDigit((char)payload[i])){
      buffer[i] = (char)payload[i];
      Serial.print(buffer[i]);
    }else{
      input_valid = false;
      Serial.println("Invalid input. New interval is not set.");
    }
  }

  if(input_valid){ 
    if(topic == mqtt_topic_interval){
      Serial.println("Input was valid. Setting interval.");
      action_interval = atoi(buffer) * 1000;
    }else if(topic == mqtt_topic_execute_length){
      Serial.println("Input was valid. Setting length.");
      action_length = atoi(buffer) * 1000; 
    }else{
      Serial.println("Topic is not configured to set anything.");
    }
  }else{
    input_valid = false;
    Serial.println("Input didn't pass validation.");
  }
}

void init_relay(){
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT); 
  pinMode(relayPin, OUTPUT);  
}

void execute_action(){
  Serial.println("Starting action.");
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(relayPin, HIGH);
  action_is_executing = true;
}

void stop_action(){
  Serial.println("Stop action.");
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(relayPin, LOW);
  action_is_executing = false;
}

void setup(){
  init_relay();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop(){
  if (!client.connected()){
    reconnect();
  }
  client.loop();

  if(action_interval != 0 && action_length != 0){
    unsigned long currentMillis = millis();

    //start action
    if (!action_is_executing){
      if(currentMillis - action_interval_previousMillis > action_interval){
        execute_action();
        action_interval_previousMillis = currentMillis;
        action_execute_previousMillis = currentMillis;
      }
    }else{
      //execute action
      if(currentMillis - action_execute_previousMillis > action_length){   
        stop_action();
        action_interval_previousMillis = currentMillis;
        action_execute_previousMillis = currentMillis;
      }
    }
  }
}

