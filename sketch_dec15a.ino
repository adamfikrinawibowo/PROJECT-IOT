#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define SS_PIN D4
#define RST_PIN D3

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

// Ganti dengan kredensial Wi-Fi Anda
const char* ssid = "adamfikri";
const char* password = "12345678";

// URL server PHP
const char* serverUrl = "http://192.168.137.1/rfid_system/register_nfc.php";

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  // Koneksi ke Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  Serial.println("Silakan scan chip NFC untuk mendaftarkan buku.");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String nfcID = getNFC(rfid.uid.uidByte, rfid.uid.size);

  Serial.print("NFC ID terdeteksi: ");
  Serial.println(nfcID);

  Serial.println("Masukkan nama buku: ");
  String namaBuku = getInputFromSerial();

  // Set stok buku menjadi 1 setiap kali scan
  String stokBuku = "1";  // Set stok default menjadi 1

  kirimDataKeServer(nfcID, namaBuku, stokBuku);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

String getNFC(byte *buffer, byte bufferSize) {
  String nfcID = "";
  for (byte i = 0; i < bufferSize; i++) {
    if (buffer[i] < 0x10) nfcID += "0";
    nfcID += String(buffer[i], HEX);
  }
  nfcID.toUpperCase();
  return nfcID;
}

String getInputFromSerial() {
  String input = "";
  while (Serial.available() == 0) {}
  input = Serial.readString();
  input.trim();
  return input;
}

void kirimDataKeServer(String nfcID, String namaBuku, String stokBuku) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClient client;

    String httpRequestData = "nfcID=" + nfcID + "&namaBuku=" + namaBuku + 
                             "&stokBuku=" + stokBuku;

    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("Response dari server: ");
      Serial.println(response);
    } else {
      Serial.print("Error saat mengirim data: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi tidak terhubung.");
  }
}
