#include <SoftwareSerial.h>

// RX di pin 5, TX di pin 4
SoftwareSerial sim800(5, 4);

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);
  
  Serial.println("Memulai tes SIM800L...");
  delay(1000);

  // Kirim perintah AT
  sim800.println("AT");
  delay(1000);
  bacaRespon();
  
  // Cek sinyal
  sim800.println("AT+CSQ");
  delay(1000);
  bacaRespon();
  
  // Cek status SIM
  sim800.println("AT+CPIN?");
  delay(1000);
  bacaRespon();
  
  // Cek registrasi jaringan
  sim800.println("AT+CREG?");
  delay(1000);
  bacaRespon();
}

void loop() {
  // Tidak ada proses di loop
}

void bacaRespon() {
  while (sim800.available()) {
    Serial.write(sim800.read());
  }
}
