#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define RED D0
#define GREEN D1

void setup() {
    // Serial Communication
    Serial.begin(9600);

    // Debug PINS
    pinMode(RED, OUTPUT);
    digitalWrite(RED, HIGH);
    pinMode(GREEN, OUTPUT);
    digitalWrite(GREEN, LOW);

    // Wifi set-up
    WiFi.mode(WIFI_STA);
    WiFi_connection();
}

void WiFi_connection() {

    // A se schimba in functie de conexiunea la internet
    const char *ssid = "iPhone-MadaZ";
    const char *password = "madal1234";

    WiFi.begin(ssid, password);

    // Metoda blocanta pentru conectare la internet
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // Se aprinde LED ul verde la conectare
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, HIGH);

    Serial.println("WiFi Connected!");
}


/* 
  Realizeaza conexiunea ESP -> Serviu IFTTT -> Notificare in aplicatie
  folosind protocolul HTTP.
  Returneaza true, daca notificarea a fost trimisa cu success
  Returneaza false, daca cererea a esuat
*/
bool send_notification(String message) {
    String IFTTT_URL = "http://maker.ifttt.com/trigger/checkPlant/with/key/f_JItHGZ7-OjVFJIw8OyXB9JgdGJX8RWBYLHc0RQirF";
    String value1 = "?value1=";
    String value2 = "&value2=https://empire-s3-production.bobvila.com/articles/wp-content/uploads/2017/09/The-Dos-and-Donts-of-Watering-Plants.jpg";
    String value3 = "&value3=";
    String send_message = value1 + value2 + value3;

    WiFiClient client;
    HTTPClient http;

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.begin(client, IFTTT_URL+send_message);
  
    // Cerere de tip POST la serverul ce face trigger appletului din aplicatie ITTT
    int httpCode = http.POST("");
    if (httpCode > 0) {

        // Pentru debug se poate afisa payload-ul
        if (httpCode == HTTP_CODE_OK) {
          const String& payload = http.getString();
        }
        return true;
    }
    return false;
}

void loop() {
    bool notification = false;

    // Se va trimite o singura notificare atunci cand ESP-ul primeste date
    // de pe interfata seriala de la Arduino UNO
    while(Serial.available() == false);

    //  Se citeste valoarea de pe seriala - niveul de umiditate
    if (WiFi.status() == WL_CONNECTED) {
        String message = Serial.readStringUntil('\0');

        // Se incearca procesul de notificare
        if (notification == false) {
            notification = send_notification(message);
        }
    }
}