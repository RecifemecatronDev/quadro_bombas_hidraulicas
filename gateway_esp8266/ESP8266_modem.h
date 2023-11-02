#ifndef ESP8266_MODEM_H
#define ESP8266_MODEM_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

void inicializacao();
String getHTML();
void saveCredentials(const String &ssid, const String &password);
void loadCredentials(String &ssid, String &password);
void startWebServer();
void setup_wifi(AsyncWebServer &server);
void atualiza_sevidor_web(AsyncWebServer &server, int &counter);

#endif