/*
  AWS IoT WiFi

  This sketch securely connects to an AWS IoT using MQTT over WiFi.
  It uses a private key stored in the ATECC508A and a public
  certificate for SSL/TLS authetication.

  It publishes a message every 5 seconds to arduino/outgoing
  topic and subscribes to messages on the arduino/incoming
  topic.

  The circuit:
  - Arduino MKR WiFi 1010 or MKR1000

  The following tutorial on Arduino Project Hub can be used
  to setup your AWS account and the MKR board:

  https://create.arduino.cc/projecthub/132016/securely-connecting-an-arduino-mkr-wifi-1010-to-aws-iot-core-a9f365

  This example code is in the public domain.
*/

#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h> // change to #include <WiFi101.h> for MKR1000

#include "arduino_secrets.h"

// 충격 센서 PIN 설정
#define SHOCK_PIN 2

// 조도 센서
#define OPEN_PIN A0

#define LED_1_PIN 5

#include <ArduinoJson.h>
#include "Led.h"

#include "Shock.h"

#include "Open.h"

/////// Enter your sensitive data in arduino_secrets.h
const char ssid[]        = SECRET_SSID;
const char pass[]        = SECRET_PASS;
const char broker[]      = SECRET_BROKER;
const char* certificate  = SECRET_CERTIFICATE;

WiFiClient    wifiClient;            // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient); // Used for SSL/TLS connection, integrates with ECC508
MqttClient    mqttClient(sslClient);

unsigned long lastMillis = 0;

Led led1(LED_1_PIN);
Shock shock1(SHOCK_PIN);
Open open1(OPEN_PIN);

int shockVal;
int openReading;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!ECCX08.begin()) {
    Serial.println("No ECCX08 present!");
    while (1);
  }

  // Set a callback to get the current time
  // used to validate the servers certificate
  ArduinoBearSSL.onGetTime(getTime);

  // Set the ECCX08 slot to use for the private key
  // and the accompanying public certificate for it
  sslClient.setEccSlot(0, certificate);

  // Optional, set the client id used for MQTT,
  // each device that is connected to the broker
  // must have a unique client id. The MQTTClient will generate
  // a client id for you based on the millis() value if not set
  //
  // mqttClient.setId("clientId");

  // Set the message callback, this function is
  // called when the MQTTClient receives a message
  mqttClient.onMessage(onMessageReceived);    // MQTT Client로부터 메시지를 받는다.
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();  // WiFi를 연결한다.
  }

  if (!mqttClient.connected()) {
    // MQTT client is disconnected, connect
    connectMQTT();  // MQTT Client와 연결한다.
  }

  // poll for new MQTT messages and send keep alives
  mqttClient.poll();

  // publish a message roughly every 5 seconds.
  // 5초마다 아두이노(디바이스)의 상태 정보를 얻어와 AWS IoT에 보낸다.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    char payload[512];
    getDeviceStatus(payload); // 아두이노(디바이스)의 상태 정보를 얻어온다.
    sendMessage(payload);
  }
}

unsigned long getTime() {
  // get the current time from the WiFi module  
  return WiFi.getTime();
}

void connectWiFi() {
  Serial.print("Attempting to connect to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the network");
  Serial.println();
}

// MQTT Client와 연결한다.
void connectMQTT() {
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println();

  Serial.println("You're connected to the MQTT broker");
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe("$aws/things/SmartSAFE/shadow/update/delta");
}

// 아두이노의 상태 정보를 얻어온다.
void getDeviceStatus(char* payload) {
  // Read led status
  const char* led = (led1.getState() == LED_ON)? "ON" : "OFF";
  // 충격 센서 실행 여부
  const char* shockRun = (shock1.getRunState() == SHOCK_RUN)? "RUN" : "STOP";
  
  shockVal = digitalRead(SHOCK_PIN);
  Serial.println(shockVal);

  // 앱에서 충격 감지 센서를 실행시킬 때만 충격을 감지
  if(shockRun == "RUN"){
    // 충격 감지 시
    if(shockVal == HIGH){
      led1.on();
      shock1.on();
    } else {
      shock1.off();
      led1.off();
    }
  }

  openReading = analogRead(OPEN_PIN);

  //int openVal = map(openReading, 0, 1024, 0, 255);   // 읽어온 데이터를 0~255범위로 변경하여 줍니다.
  Serial.println(openReading);
  // 금고를 열 때
  if(openReading < 500) {
    open1.openSafe();
  } else {
    open1.closeSafe();
  }
  const char* openSafe = (open1.getState() == OPEN)? "OPEN" : "CLOSE";

  // 충격 감지시 ON
  const char* shock = (shock1.getState() == SHOCK_ON)? "ON" : "OFF";
  
  // make payload for the device update topic ($aws/things/SmartSAFE/shadow/update)
  sprintf(payload,"{\"state\":{\"reported\":{\"SHOCK_RUN\":\"%s\",\"LED\":\"%s\", \"SHOCK\":\"%s\",\"SAFE\":\"%s\"}}}",shockRun,led,shock,openSafe);
}

// MQTT Client에 메시지를 보낸다.
void sendMessage(char* payload) {
  char TOPIC_NAME[]= "$aws/things/SmartSAFE/shadow/update";
  
  Serial.print("Publishing send message:");
  Serial.println(payload);
  mqttClient.beginMessage(TOPIC_NAME);
  mqttClient.print(payload);
  mqttClient.endMessage();
}

// AWS IoT로부터 메시지를 받는다.
void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // store the message received to the buffer
  char buffer[512] ;
  int count=0;
  while (mqttClient.available()) {
     buffer[count++] = (char)mqttClient.read();
  }
  buffer[count]='\0'; // 버퍼의 마지막에 null 캐릭터 삽입
  Serial.println(buffer);
  Serial.println();


  // JSon 형식의 문자열인 buffer를 파싱하여 필요한 값을 얻어옴.
  // 디바이스가 구독한 토픽이 $aws/things/MyMKRWiFi1010/shadow/update/delta 이므로,
  // JSon 문자열 형식은 다음과 같다.
  // {
  //    "version":391,
  //    "timestamp":1572784097,
  //    "state":{
  //        "LED":"ON"
  //        "SHOCK_RUN:"RUN"
  //    },
  //    "metadata":{
  //        "LED":{
  //          "timestamp":15727840
  //         },
  //        "SHOCK_RUN":{
  //          "timestamp":15727840
  //         }
  //    }
  // }
  //
    
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, buffer);
  JsonObject root = doc.as<JsonObject>();
  JsonObject state = root["state"];
  //const char* led = state["LED"];
  const char* shockRun = state["SHOCK_RUN"];
  //const char* shock = state["SHOCK"];
  //const char* openSafe = state["SAFE"];
  //Serial.println(led);
  Serial.println(shockRun);
  //Serial.println(shock);
  //Serial.println(openSafe);
   
  char payload[512];
 

   if (strcmp(shockRun,"RUN")==0) {
    shock1.runShock();
    sprintf(payload,"{\"state\":{\"reported\":{\"SHOCK_RUN\":\"%s\"}}}","RUN");
    sendMessage(payload);
    
  } else if (strcmp(shockRun,"STOP")==0) {
    shock1.stopShock();
    sprintf(payload,"{\"state\":{\"reported\":{\"SHOCK_RUN\":\"%s\"}}}","STOP");
    sendMessage(payload);
  }
  /*

  // 충격 센서 테스트
  if (strcmp(shock,"ON")==0) {
    shock1.on();
    sprintf(payload,"{\"state\":{\"reported\":{\"SHOCK\":\"%s\"}}}","ON");
    sendMessage(payload);
    
  } else if (strcmp(shock,"OFF")==0) {
    shock1.off();
    sprintf(payload,"{\"state\":{\"reported\":{\"SHOCK\":\"%s\"}}}","OFF");
    sendMessage(payload);
  }
  */
     /*
  if (strcmp(openSafe,"OPEN")==0) {
    led1.on();
    sprintf(payload,"{\"state\":{\"reported\":{\"LED\":\"%s\"}}}","ON");
    sendMessage(payload);
    
  } else if (strcmp(openSafe,"CLOSE")==0) {
    led1.off();
    sprintf(payload,"{\"state\":{\"reported\":{\"LED\":\"%s\"}}}","OFF");
    sendMessage(payload);
  }*/
}
