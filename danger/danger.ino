#define USE_ARDUINO_INTERRUPTS true    
#include <PulseSensorPlayground.h>
#include "VoiceRecognitionV3.h"
#include <SoftwareSerial.h>

#define TEMP_CORRECTION 13

const int PulseWire = 0;
const int LED13 = 13;
int Threshold = 550;

unsigned long previousMillis = 0;
const int times = 1;//초 설정

int value;
int tmp_sensor = A1;
float voltage;
float temperatureC;

bool danger = false;
bool problem = false;

char speaker[1];

uint8_t buf[64];

VR myVR(2,3);
SoftwareSerial SpeakerInstruct(4,5);

PulseSensorPlayground pulseSensor;
void setup() {
  Serial.begin(9600);
  myVR.begin(9600);
  SpeakerInstruct.begin(9600);

  pulseSensor.analogInput(PulseWire);   
  pulseSensor.blinkOnPulse(LED13);       
  pulseSensor.setThreshold(Threshold);   

  pinMode(tmp_sensor, INPUT);

  if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");  
  }
}

String padStart(String input, int targetLength, char padChar = '0') {
  while (input.length() < targetLength) {
    input = padChar + input;
  }
  return input;
}

void loop() {
  unsigned long currentMillis = millis();

  if(Serial.available()>0){
    char reads = Serial.read();
    if(reads == 'E'){
      danger = true;
    }
    SpeakerInstruct.write('t');
    int ret;
    buf[1] = -1; // 초기화
    while (millis() - currentMillis < 10000) {//10 초간 대답 돌아오길 기다리기
      ret = myVR.recognize(buf, 50);
      if(ret > 0 && buf[1] != -1){
        break;
      }
    }

    if(buf[1] == -1){// 사용자가 문제가 생긴 경우
      speaker[0] = 'D';
      SpeakerInstruct.write(speaker);
      speaker[0] = '0';
    }
  }
  if(currentMillis - previousMillis >= 1000*times){
    value = analogRead(tmp_sensor); // 체온 체크
    voltage = value * 5.0 / 1023.0;
    temperatureC = voltage / 0.01;
    temperatureC -= TEMP_CORRECTION;

    int myBPM = pulseSensor.getBeatsPerMinute();

    if (pulseSensor.sawStartOfBeat()) { // 심박 데이터체크
      Serial.println("A HeartBeat Happened");
      Serial.print("BPM: ");
      Serial.println(myBPM);         
    }
    
    Serial.print("temperature :");
    Serial.print(temperatureC);
    Serial.println("C");

    String send_temperatureC = padStart(String(temperatureC, 1),5, '0');
    String send_BPM = padStart(String(myBPM),3, '0');
    String send_data = send_BPM + " " + send_temperatureC;
    sendInChunks(send_data);

    previousMillis = currentMillis;
  }
  int ret = myVR.recognize(buf, 50);
  if(ret>0){
    switch(buf[1]){
      case (0): // 도와줘
      case (1): // 살려줘
      case (2): // 구해줘
      case (5): // 아파
        problem = true;
        speaker[0] = (buf[1] == 0) ? 'h' :
                    (buf[1] == 1) ? 'f' :
                    (buf[1] == 2) ? 'w' : 's';
        break;
      case (3)://어
        if(problem == true){
          speaker[0] = 'D';
        }
        else{
          speaker[0] = '0';
        }
        break;
      case (4)://아니
        problem = false;
        speaker[0] = '0';
        break;
    }
    if(speaker[0] !='0'){
      SpeakerInstruct.write(speaker);
      Serial.println(speaker);
    }
  }
}

void sendInChunks(String data) {
  int chunkSize = 10;
  int length = data.length();

  for (int i = 0; i < length; i += chunkSize) {
    String chunk = data.substring(i, i + chunkSize);
    Serial.write(chunk.c_str()); 
    Serial.flush(); // 송신이 끝날 때까지 기다림
    delay(150);     // 충분한 대기시간 (기존 100ms에서 늘림)
  }
}

