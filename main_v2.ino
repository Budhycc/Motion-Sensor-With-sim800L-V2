#include <SoftwareSerial.h>

// Pin konfigurasi
#define PIR_PIN 6
#define LED_PIN 7
#define READY_LED_PIN 8  // LED indikator sistem siap
#define SIM800_TX 5
#define SIM800_RX 4

// Inisialisasi komunikasi ke SIM800L
SoftwareSerial sim800(SIM800_TX, SIM800_RX);

const char phoneNumber[] = "+6289526808456";  // Ganti dengan nomor tujuan
bool alreadyCalled = false;
bool simReady = false;

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(READY_LED_PIN, OUTPUT);  // Inisialisasi pin LED sistem siap
  digitalWrite(LED_PIN, LOW);
  digitalWrite(READY_LED_PIN, LOW); // Awalnya mati

  Serial.begin(9600);
  sim800.begin(9600);
  Serial.println("Memulai validasi SIM800L...");

  // Ulangi pengecekan SIM800L hingga berhasil
  while (!simReady) {
    simReady = cekModulSiap();
    if (!simReady) {
      Serial.println("SIM800L tidak siap. Mencoba lagi dalam 5 detik...");
      delay(5000);
    } else {
      Serial.println("SIM800L siap digunakan.");
      digitalWrite(READY_LED_PIN, HIGH); // Nyalakan LED indikator
    }
  }
}

void loop() {
  if (!simReady) return;

  int motion = digitalRead(PIR_PIN);

  if (motion == HIGH && !alreadyCalled) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Gerakan terdeteksi!");
    callPhoneNumber(phoneNumber);
    alreadyCalled = true;
  }

  if (motion == LOW && alreadyCalled) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("Gerakan berhenti. Sistem siap.");
    alreadyCalled = false;
  }

  delay(500); // Debounce PIR
}

// ==============================
// Validasi modul SIM800L
// ==============================
bool cekModulSiap() {
  if (!kirimDanCek("AT", "OK")) return false;
  if (!kirimDanCek("AT+CSQ", "+CSQ")) return false;
  if (!kirimDanCek("AT+CPIN?", "READY")) return false;

  String resp = kirimDanAmbil("AT+CREG?");
  if (!(resp.indexOf("+CREG: 0,1") != -1 || resp.indexOf("+CREG: 0,5") != -1)) {
    Serial.println("Registrasi jaringan gagal.");
    return false;
  }

  return true;
}

bool kirimDanCek(const char* perintah, const char* keyword) {
  String response = kirimDanAmbil(perintah);
  return response.indexOf(keyword) != -1;
}

String kirimDanAmbil(const char* perintah) {
  sim800.println(perintah);
  delay(1000);

  String hasil = "";
  long waktuMulai = millis();
  while (millis() - waktuMulai < 3000) {
    while (sim800.available()) {
      char c = sim800.read();
      hasil += c;
    }
  }

  Serial.print("Respon: ");
  Serial.println(hasil);
  return hasil;
}

// ==============================
// Fungsi melakukan panggilan
// ==============================
void callPhoneNumber(const char* number) {
  Serial.print("Memanggil ");
  Serial.println(number);

  sim800.print("ATD");
  sim800.print(number);
  sim800.println(";");

  // Tunggu sampai panggilan aktif (status CLCC = 0)
  unsigned long timeout = millis() + 15000;  // Maks 15 detik tunggu
  bool callActive = false;

  while (millis() < timeout) {
    sim800.println("AT+CLCC");
    delay(500);

    String response = "";
    while (sim800.available()) {
      char c = sim800.read();
      response += c;
    }

    Serial.print("CLCC Respon: ");
    Serial.println(response);

    // Cek status panggilan aktif: +CLCC: ...,0,...
    int clccIndex = response.indexOf("+CLCC:");
    if (clccIndex != -1) {
      int statusPos = response.indexOf(',', clccIndex);
      for (int i = 0; i < 2 && statusPos != -1; i++) {
        statusPos = response.indexOf(',', statusPos + 1);
      }

      if (statusPos != -1 && response.charAt(statusPos + 1) == '0') {
        callActive = true;
        break;
      }
    }
  }

  if (callActive) {
    Serial.println("Panggilan aktif. Menunggu 30 detik...");
    delay(30000);  // Waktu bicara
    sim800.println("ATH");
    Serial.println("Panggilan dihentikan.");
  } else {
    Serial.println("Panggilan gagal atau tidak aktif.");
    sim800.println("ATH"); // Cegah stuck
  }
}
