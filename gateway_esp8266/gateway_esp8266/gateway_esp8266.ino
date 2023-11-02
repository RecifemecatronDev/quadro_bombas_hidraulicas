#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <time.h>
#include "index_html.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -4 * 3600;  // Brasília, Brasil GMT -3
const int   daylightOffset_sec = 3600;

#define SSID_ADDR 0
#define PASSWORD_ADDR 100

const char* ap_ssid = "quadro";
const char* ap_password = "12345678";
String ssid = "";
String password = "";

#define ledNode 2

ESP8266WebServer server(80);  // Cria um servidor na porta 80

String lastMessage = "";

String printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return "";
  }
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buffer);
}

//FUNÇÃO PARA LIMPAR A EEPROM
void clearEEPROM() {
  // Tamanho total da memória EEPROM no ESP8266
  const int EEPROM_SIZE = 4096;

  // Inicia a EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Limpa toda a EEPROM
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }

  // Grava as mudanças
  EEPROM.commit();
}

void setupWiFiAPMode() {
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("DISPLAY AP IP address: ");
  Serial.println(myIP);
}

void setupWiFiStationMode() {

  //Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando a rede WiFi ");
  Serial.println(ssid);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED){
    //PISCA O LED DO NODE ENQUANTO NÃO CONECTA
        digitalWrite(ledNode, !digitalRead(ledNode));
    Serial.print('.');
    delay(1000);

        if (count > 5) {
      setupWiFiAPMode();
      return;
    }
    count++;

          if(WiFi.status() == WL_CONNECTED)
      {
        Serial.println();
        Serial.printf("Conectado ao SSID: %s",ssid);
        Serial.println("");
          Serial.print("DISPLAY IP address: ");
          Serial.println(WiFi.localIP());
        digitalWrite(ledNode, LOW);

      // Configura a hora NTP após a conexão WiFi ser estabelecida
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      Serial.println(printLocalTime());
      //AJUSTA A DATA E HORA DO CIRCUITO PRINCIPAL
      delay(5000);
      String comando_ajustar_hora = "DATE "+printLocalTime();
      Serial.println(comando_ajustar_hora);
      }
  }
}

// Função para ler um string da EEPROM
/*
String readStringFromEEPROM(int startAddr, int maxLength) {
  String readString;
  EEPROM.begin(maxLength); 

  for (int i = 0; i < maxLength; ++i) {
    char read = EEPROM.read(startAddr + i);
    if (read == 0) {
      break;
    }
    readString += read;
  }

  return readString;
}
*/

// Função para escrever um string na EEPROM
/*
void writeStringToEEPROM(int startAddr, const String &writeString, int maxLength) {
  EEPROM.begin(maxLength);

  int len = writeString.length();
  len = min(len, maxLength);

  for (int i = 0; i < len; ++i) {
    EEPROM.write(startAddr + i, writeString[i]);
  }

  EEPROM.write(startAddr + len, 0); // Adiciona o terminador nulo

  EEPROM.commit();
}
*/

void loadCredentials(String &ssid, String &password) {
  /*
  ssid = readStringFromEEPROM(SSID_ADDR, 100);
  password = readStringFromEEPROM(PASSWORD_ADDR, 100);
  */
  EEPROM.begin(512); //Initialize EEPROM

  ssid = "";
  password = "";

  // Carrega a SSID
  for (int i = 0; i < 512; ++i) {
    char read = EEPROM.read(i);

    if (read == 0) {
      break;
    }

    ssid += read;
  }

  // Carrega a senha após a SSID
  for (int i = 0; i < 512; ++i) {
    char read = EEPROM.read(ssid.length() + i + 1);

    if (read == 0) {
      break;
    }

    password += read;
  }
  
}

void saveCredentials(const String &ssid, const String &password) {
  /*writeStringToEEPROM(SSID_ADDR, ssid, 100);
  writeStringToEEPROM(PASSWORD_ADDR, password, 100);*/
  
  EEPROM.begin(512); //Initialize EEPROM

  // Salva a SSID
  for (int i = 0; i < ssid.length(); ++i) {
    EEPROM.write(i, ssid[i]);
  }

  // Salva a senha após a SSID
  for (int i = 0; i < password.length(); ++i) {
    EEPROM.write(ssid.length() + i + 1, password[i]);
  }

  // Terminate strings with 0
  EEPROM.write(ssid.length(), 0);
  EEPROM.write(ssid.length() + password.length() + 1, 0);

  EEPROM.commit(); //Make sure to commit changes to EEPROM
  
}

void setup() {
  //clearEEPROM();
  Serial.begin(9600);
  delay(10);

  pinMode(ledNode, OUTPUT);
///////////////////////////////////////////////////////////////////////////////////////////////////////////
    EEPROM.begin(512);  // Inicializa a EEPROM com o tamanho do espaço de armazenamento que você deseja

  loadCredentials(ssid, password);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Tenta conectar o ESP8266 a uma rede WiFi existente
  // se não for bem sucedido, configura o ESP8266 como Access Point
  setupWiFiStationMode();

  server.on("/", []() {
    server.send(200, "text/html", index_html);
  });

  // ... Adicione mais rotas aqui conforme necessário...
    server.on("/command", []() {
    String command = server.arg("command");

    Serial.println(command);  // Envia o comando para o Arduino Mega

    server.send(200, "text/plain", "Command sent: " + command);
  });

  server.on("/lastMessage", []() {
    server.send(200, "text/plain", lastMessage);
  });

  server.on("/setWiFi", HTTP_POST, []() {
    String ssid_new = server.arg("ssid");
    String password_new = server.arg("password");

    saveCredentials(ssid_new, password_new);

    server.send(200, "text/plain", "Credenciais de Wifi atualizadas. Reinicializando sistema.");
    delay(2000);
    ESP.restart();
});

server.on("/scanWiFi", HTTP_GET, []() {
    String wifiList = "";
    int n = WiFi.scanNetworks();

  String json = "[";

  for (int i = 0; i < n; ++i) {
    if (i > 0) {
      json += ",";
    }
    json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  json += "]";

    /*
    Serial.println("scan done");
    if (n == 0) {
        wifiList = "no networks found";
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            wifiList += String(i + 1) + ": " + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ")\n";
            delay(10);
        }
    }
    */

    wifiList = json;
    server.send(200, "text/plain", wifiList);
});

  server.begin();
  Serial.println("Servidor iniciado");
}

void loop() {
  server.handleClient();  // Manipula solicitações do cliente

    // Verifica se há novas mensagens do Arduino
  while (Serial.available()) {
    String message = Serial.readString();
    lastMessage = message;
  }
}