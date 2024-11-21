#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

// Ganti dengan SSID dan password Wi-Fi Anda
const char* ssid = "aa";
const char* password = "12345678";
// Ganti dengan URL Supabase API dan API Key
const String supabaseURL = "https://iqdnpgggupisxhhfepbi.supabase.co";
const String apiKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImlxZG5wZ2dndXBpc3hoaGZlcGJpIiwicm9sZSI6ImFub24iLCJpYXQiOjE3MzE0NzE3NDEsImV4cCI6MjA0NzA0Nzc0MX0.s2aDzTXB0Q4OQF9YSO8amZ8syCVkCNWU5ORfLf_v8l0";

// Bot Tele
const char *BOT_TOKEN = "7595832507:AAGPD5WfYH3C14qVCe8344gg-_XYZ5KJsm8";
const char *CHAT_ID = "7843507337";

// Library Sensor asap
#define SENSOR_PIN A0
// Interval pengiriman data ke Supabase dalam milidetik
int sendingInterval = 3000; // Pengiriman data setiap 10 detik
HTTPClient https;
WiFiClientSecure client;

void setup(){
  // Mengatur pin LED bawaan untuk menunjukkan status pengiriman data
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // LED menyala saat tidak mengirim data

  // Menghubungkan ke Wi-Fi
  Serial.begin(9600);

  connectToWiFi();
  // HTTPS tanpa validasi sertifikat
  client.setInsecure();

 
}

void loop(){
  // Cek status koneksi Wi-Fi
  if (WiFi.status() == WL_CONNECTED)  {
    digitalWrite(LED_BUILTIN, LOW); // LED menyala saat mengirim data

    // kirim notif ke tele
     int moistureValue = readSoilMoisture();
    if (moistureValue > 600){
      sendTelegramMessage("Hello,  lembab terdeteksi tinggi!!!!");
    }

    // Mengirim data ke Supabase
    sendToSupabase(moistureValue);

    // Matikan LED setelah pengiriman data
    digitalWrite(LED_BUILTIN, HIGH); // LED mati
  }
  else{
    Serial.println("Error in Wi-Fi connection");
  }

  // Tunggu beberapa detik sebelum membaca dan mengirim data lagi
  delay(sendingInterval); // 3 detik sebelum membaca sensor lagi
}

// telegram
void sendTelegramMessage(String message){
  if (WiFi.status() == WL_CONNECTED)  {                          // Pastikan sudah terhubung ke WiFi
    // Format URL Telegram API
    String url = "https://api.telegram.org/bot" + String(BOT_TOKEN) + "/sendMessage?chat_id=" + String(CHAT_ID) + "&text=" + message;
    https.begin(client, url); 

    // Lakukan GET request ke Telegram API
    int httpCode = https.GET();
    // Periksa kode respon HTTP
    if (httpCode > 0)    {
        //      Serial.println("HTTP Response Code: " + String(httpCode));
        String payload = https.getString();
        Serial.println("Respon Telegram       : " + payload);
    }
    else{
      Serial.println("Error Mengirim Tele  : " + String(httpCode));
    }
    https.end(); // Menutup koneksi
  }
  else{
    Serial.println("WiFi not connected");
  }
}

// Fungsi untuk menghubungkan ke Wi-Fi
void connectToWiFi() {
 Serial.print("Connecting to Wi-Fi");
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }
 Serial.println("");
 Serial.println("Wi-Fi connected.");
 Serial.println("IP address: ");
 Serial.println(WiFi.localIP());
}

int readSoilMoisture() {
 int moistureValue = analogRead(SENSOR_PIN);
 int moisture = 1024 - moistureValue; // Biasanya 1024 - nilai sensor untuk kelembapan
 Serial.print("Soil Moisture: ");
 Serial.println(moisture);
 return moisture;
}
// Fungsi untuk mengirim data ke Supabase
void sendToSupabase(int moistureValue) {
 // Menyusun data JSON untuk dikirim
 String postData = "{\"kelembapan\":" + String(moistureValue) + "}";
 // Mengirim permintaan POST ke Supabase
 https.begin(client, supabaseURL + "/rest/v1/tanah");
 https.addHeader("Content-Type", "application/json");
 https.addHeader("Prefer", "return=representation");
 https.addHeader("apikey", apiKey);
 https.addHeader("Authorization", "Bearer " + apiKey);
 // Mengirim data JSON
 int httpCode = https.POST(postData);
 String payload = https.getString(); 
 // Menampilkan hasil HTTP
 Serial.println(httpCode);
 Serial.println(payload);
 https.end(); // Menutup koneksi
} 
