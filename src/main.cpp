#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char *ssid = "YOUR-SSID";
const char *password = "YOUR-PASSWORD";
const char *mqtt_server = "broker.mqtt-dashboard.com";
 
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
long int value = 0;
const int relayPin = D1;
const int ledPin = D2;
const long interval = 5000;

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
      client.subscribe("relaytestj");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void callback(char *topic, byte *payload, signed int length){
  Serial.print("Message arrived: ");
  Serial.println(topic);
  for (int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if ((char)payload[0] == '1'){
    digitalWrite(LED_BUILTIN, LOW);
  } else{
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

void initRelay(){
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT); 
  pinMode(relayPin, OUTPUT);  
}

void setup(){
  initRelay();
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
}