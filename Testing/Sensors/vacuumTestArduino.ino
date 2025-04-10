#include <SoftwareSerial.h>
#include <MS5611_SPI.h>
#define RX_RYLR 6
#define TX_RYLR 7

#define MS5611_CHIPSELECT     10
#define MSLPRESSURE  101325.0

SoftwareSerial RYLR (RX_RYLR, TX_RYLR);
MS5611_SPI MS5611(10, 11, 12, 13);   // arduino uno spi pinout

String packet, transmit;

int count=0;

float prevPressure, pressure, temperature, altitude;
String detect;

void sendData()
{
  packet="";
  packet+=String(millis())+",";
  packet+=String(pressure) +","+String(temperature)+","+String(altitude)+","+detect;
  transmit = "AT+SEND=0,"+String(packet.length())+","+packet+"\r\n";
  RYLR.print(packet);
  Serial.println("txed-");
}

void setup() {
  Serial.begin(9600);
  RYLR.begin(57600);

  // MS5611.begin();
  if (MS5611.begin() == true) {
    Serial.println("\t...MS5611 Initialized!");
  }
  else
    Serial.println("MS5611 not init");
  int result = MS5611.read();
  prevPressure = MS5611.getPressure();

  packet = "AT+SEND=0,5,HELLO\r\n";
  Serial.print(packet);

  Serial.println("setup");
}

void loop() {
  //debugging code 
  // if(Serial.available()) {
  //   // String data = Serial.readStringUntil('\n');
  //   // sendData(data);
  //   transmit = "AT+SEND=0," + String(data.length()) + "," + data + "\r\n";
  // RYLR.print(transmit);
  // delay(10);
  //   Serial.println("TRANSMITTED: " + transmit);
  // }

  int result = MS5611.read(); //do not comment this shit out 
  if (result != MS5611_READ_OK) //idk what it does but it doesnt work without it (MS gives buffer values -9.9 for pressure)
  {
    Serial.print("Error in read: "); 
    Serial.println(result);
  }

  pressure = MS5611.getPressure();
  temperature = MS5611.getTemperature();
  altitude = 44330.0*(1.0 - pow((float)(pressure*100)/MSLPRESSURE, 0.1902949));
  
  int flag=0;
  if(pressure-prevPressure>1.5)
    count++;
  if(flag==0&&count>10)//apogeee //drogur not done+10 increasing readings 
  {
    // digitalWrite(DROGUE, HIGH);
    // detect="DROGUE";
    flag=1; //drogueee
  }
  if(flag==3 && altitude<=457)
  {
    // digitalWrite(MAIN, HIGH);
    // detect="MAIN";
    // flag=1;
    flag=2; //mainnn
  }

  prevPressure=pressure;
  // sendData(); //will also most likely work, didnt for a bit and started again, the same contents are written below in main 

  if(flag==1) //drogue detected
    detect="DROGUE"; 
  if (flag==2) //main detected
    detect="MAIN";
  packet="";
  packet+=String(millis())+",";
  packet+=String(pressure) +","+String(temperature)+","+String(altitude)+","+detect;
  transmit = "AT+SEND=0," + String(packet.length()) + "," + packet + "\r\n";
  RYLR.print(transmit);
  delay(10);
    Serial.println("TRANSMITTED: " + transmit);

  Serial.println("Pressure: "+String(pressure)+", Temp: "+String(temperature)+", Altitude: "+String(altitude)+", Detect: "+detect);
  delay(1000);
  if(flag==1) //drogue sent
    flag=3; //drogue done
  if (flag==2) //main snet
    flag=4; //main done
  detect=""; //everything else is just plain string
  
}
