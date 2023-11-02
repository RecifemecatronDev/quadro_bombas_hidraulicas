#include "ESP8266_modem.h"

#ifdef ESP32
  #include <WiFi.h>
  #include <SPIFFS.h>
  #include <AsyncTCP.h>
  #include <HTTPClient.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <FS.h>
  #include <ESPAsyncTCP.h>
  #include <ESP8266HTTPClient.h>
#endif


#include <EEPROM.h>
#include <ESPAsyncWebServer.h>

bool useAsyncWebServer = true;

void inicializacao() {
  EEPROM.begin(512);
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
}

String scanNetworks() {
  int numNetworks = WiFi.scanNetworks();
  String json = "[";

  for (int i = 0; i < numNetworks; ++i) {
    if (i > 0) {
      json += ",";
    }
    json += "{\"ssid\":\"" + WiFi.SSID(i) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
  }
  json += "]";

  return json;
}

String readHTMLfromSPIFFS(const char* filePath) {
  if (!SPIFFS.exists(filePath)) {
    Serial.printf("File '%s' does not exist\n", filePath);
    return "";
  }

  File file = SPIFFS.open(filePath, FILE_READ);

  if (!file) {
    Serial.printf("Failed to open file '%s' for reading\n", filePath);
    return "";
  }

  String htmlContent = file.readString();
  file.close();

  return htmlContent;
}

String getHTML() {

String html = "<!DOCTYPE html><html><head><title>Configurar Wi-Fi</title>";
  html += "<script>";
  html += "function fetchNetworks() {";
  html += "  fetch('/scanNetworks')";
  html += "    .then(response => response.json())";
  html += "    .then(data => {";
  html += "      var networksList = document.getElementById('networksList');";
  html += "      networksList.innerHTML = '';";
  html += "      data.forEach(network => {";
  html += "        var listItem = document.createElement('li');";
  html += "        listItem.textContent = network.ssid + ' (' + network.rssi + ' dBm)';";
  html += "        networksList.appendChild(listItem);";
  html += "      });";
  html += "    });";
  html += "}";
  html += "</script>";
  html += "</head><body>";
  html += "<h1>Configurar Wi-Fi</h1>";
  html += "<form action='/setWiFi' method='post'>";
  html += "SSID:<br><input type='text' name='ssid' required><br>";
  html += "Senha:<br><input type='password' name='password'><br>";
  html += "<input type='submit' value='Conectar'>";
  html += "</form>";
  html += "<button id='scanNetworks' onclick='fetchNetworks()'>Buscar redes</button>";
  html += "<ul id='networksList'></ul>";
  html += "</body></html>";
 
  //CASO FUTURAMENTE QUEIRA LER UM ARQUIVO HTML
  //String html = readHTMLfromSPIFFS("/index.html");
  return html;
}


void saveCredentials(const String &ssid, const String &password) {
  EEPROM.writeString(0, ssid);
  EEPROM.writeString(ssid.length() + 1, password);
  EEPROM.commit();
}

void loadCredentials(String &ssid, String &password) {
  ssid = EEPROM.readString(0);
  password = EEPROM.readString(ssid.length() + 1);
}

void startAsyncWebServer(AsyncWebServer &server) {

  const char *ssidAP = "CONTADOR_DE_ACESSOS_AP";
  const char *passwordAP = "";  // Você pode definir sua própria senha aqui

  // Inicie o modo AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidAP, passwordAP);

  Serial.print("AP iniciado com SSID: ");
  Serial.println(ssidAP);
  Serial.print("Senha: ");
  Serial.println(passwordAP);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Endereço IP do AP: ");
  Serial.println(IP);

  // Configure o servidor Async Web aqui
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", getHTML());
  });

  server.on("/scanNetworks", HTTP_GET, [](AsyncWebServerRequest *request) {
  String networks = scanNetworks();
  request->send(200, "application/json", networks);
});

  server.on("/setWiFi", HTTP_POST, [](AsyncWebServerRequest *request) {
    String ssid = "";
    String password = "";

    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      ssid = request->getParam("ssid", true)->value();
      password = request->getParam("password", true)->value();

      Serial.println("Conectando ao Wi-Fi com as credenciais fornecidas...");
      WiFi.disconnect();
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid.c_str(), password.c_str());

      unsigned long startTime = millis();
      while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
        delay(100);
        Serial.print(".");
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConectado com sucesso!");
        saveCredentials(ssid, password);
        request->send(200, "text/plain", "Conectado com sucesso!");
      } else {
        Serial.println("\nFalha ao conectar. Verifique as credenciais e tente novamente.");
        request->send(200, "text/plain", "Falha ao conectar. Verifique as credenciais e tente novamente.");
      }
    } else {
      request->send(400, "text/plain", "Parâmetros inválidos.");
    }
  });

  server.begin();
  Serial.println("Servidor Async Web iniciado");
}

void startWebServer(AsyncWebServer &server) {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Servidor Async Web em execução");
  });

  server.begin();
  Serial.println("Servidor Async Web iniciado");
}

void setup_wifi(AsyncWebServer &server) {
  Serial.println("Obtendos credenciais salvas na EEPROM");
  String savedSSID, savedPassword;
  loadCredentials(savedSSID, savedPassword);

  if (savedSSID != "" && savedPassword != "") {
    Serial.println("Tentando conectar com as credenciais salvas...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
    Serial.print("Conectando ao WiFi ..");
    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {
      delay(100);
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConectado com sucesso!");
    } else {
      Serial.println("\nFalha ao conectar. Iniciando o modo AP...");
      if (useAsyncWebServer) {
        startAsyncWebServer(server);
      } else {
        startWebServer(server);
      }
    }
  } else {
    Serial.println("Nenhuma credencial salva encontrada. Iniciando o modo AP...");
    if (useAsyncWebServer) {
      startAsyncWebServer(server);
    } else {
      startWebServer(server);
    }
  }
}

void atualiza_sevidor_web(AsyncWebServer &server, int &counter) {
  server.on("/", HTTP_GET, [counter](AsyncWebServerRequest *request) {
    String html = "<html><head><meta http-equiv=\"refresh\" content=\"1\"></head><body><h1>Contador: ";
    html += String(counter);  // Adiciona valor da variável ao HTML
    html += "</h1></body><script>setTimeout(function(){ location.reload(); }, 1000);</script></html>";
    request->send(200, "text/html", html);
  });
}
