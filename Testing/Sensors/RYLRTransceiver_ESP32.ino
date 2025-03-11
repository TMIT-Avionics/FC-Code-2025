// Serial1/Serial2 can be mapped to any pin (except input only)
// RX2/TX2 in pinout doesn't restrain it to Serial2

#define RXD2 16
#define TXD2 17

String data, response, transmit, input;

String parseRYLR(String input) {
  int start = input.indexOf(',') + 1;
  start = input.indexOf(',', start) + 1;
  int end = input.indexOf(',', start);
  String parsed = input.substring(start, end);
  parsed.trim();
  return parsed;  
}

void receiveData() {
  if (Serial1.available()) {
    response = Serial1.readStringUntil('\n');
    response = parseRYLR(response);
    Serial.println("DATA RECEIVED: " + response);
    //Serial1.clear();
  }
}

void sendData(String data) {
  transmit = "AT+SEND=0," + String(data.length()) + "," + data + "\r\n";
  Serial1.print(transmit);
  delay(10);
}

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial1.begin(57600, SERIAL_8N1, RXD2, TXD2); // Serial1 for RYLR
  Serial.println("_________________");
  Serial.println("RYLR Testing Code");
  Serial.println("_________________");
}

void loop() {
  if(Serial.available()) {
    input = Serial.readStringUntil('\n');
    sendData(input);
    Serial.print("TRANSMITTED: " + transmit);
  }
  receiveData();
}