#define RXD2 7
#define TXD2 8

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
  if (Serial2.available()) {
    response = Serial2.readStringUntil('\n');
    response = parseRYLR(response);
    Serial.println("DATA RECEIVED: " + response);
    //Serial2.clear();
  }
}

void sendData(String data) {
  transmit = "AT+SEND=0," + String(data.length()) + "," + data + "\r\n";
  Serial2.print(transmit);
  delay(10);
}

void setup() {
  Serial.begin(115200);
  delay(5000);
  Serial2.begin(57600); // Serial2 for RYLR
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