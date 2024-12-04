#include <WiFiS3.h>

char ssid[] = " ";
char pass[] = " ";

int status = WL_IDLE_STATUS;
char server[] = "192.168.171.2";

WiFiClient wifiClient;

const int chunkSize = 10;
char buffer[chunkSize];
int index = 0;

String chunk = "000 000.0";

uint8_t receive = 0;

void setup() {
  Serial1.begin(9600);
  Serial.begin(9600);
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(10000);
  }

  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  if (wifiClient.connect("server", 8030)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    // wifiClient.println("GET /tables/list/ HTTP/1.1");
    wifiClient.println("GET / HTTP/1.1");
    wifiClient.println("Host: " + String(server));
    wifiClient.println("Connection: close");
    wifiClient.println();
  }
}

void loop() {
  // read_response();

  if (Serial1.available() > 0) {
    while (Serial1.available() > 0) {
      char receivedChar = Serial1.read(); // 한 바이트씩 읽음
      if (index < 10) { // buffer 크기 제한
        buffer[index++] = receivedChar;
      }

      // 청크가 완전히 수신된 경우
      if (index == 10) {
        buffer[index] = '\0'; // 문자열 종료
        Serial.println("수신된 데이터: " + String(buffer)); // 디버깅 출력
        index = 0; // 버퍼 초기화
      }
    }
    if (wifiClient.connected()) {
      wifiClient.println(chunk);
    }
  }


  if (!wifiClient.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    wifiClient.stop();
    while (true);
  }
  delay(100);
}

void printWifiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void read_response() {
  uint32_t received_data_num = 0;
  while (wifiClient.available()) {
    char c = wifiClient.read();
    receive = static_cast<int>(c);
  }
  if(receive == 1){
    Serial.write("E");
    receive = 0;
  }
}