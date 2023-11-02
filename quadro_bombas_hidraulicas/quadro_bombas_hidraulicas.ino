#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>

#define NUM_BOMBAS 6

//const int PINS_BOMBAS[NUM_BOMBAS] = {D3, D4, D5, D6, D7, D0};
const int PINS_BOMBAS[NUM_BOMBAS] = {7, 8, 9, 10, 11, 12};

RTC_DS1307 rtc;

bool status_bombas[NUM_BOMBAS];

LiquidCrystal_I2C lcd(0x27, 20, 4);

String command = "";

int dayOfYear(int month, int day, int year) {
    int monthDays[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    // Ajusta para anos bissextos
    if (month > 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        return monthDays[month - 1] + day + 1;
    }
    
    return monthDays[month - 1] + day;
}

void setup() {

      for (int i = 0; i < NUM_BOMBAS; i++) {
    pinMode(PINS_BOMBAS[i], OUTPUT);
    digitalWrite(PINS_BOMBAS[i], HIGH);
    status_bombas[i] = false;
  }

  Wire.begin();
  rtc.begin();
  Serial.begin(9600); // Inicia a comunicação serial com velocidade de 9600 baud
  
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(2023, 6, 22, 12, 0, 0));
  }
  
  lcd.begin(20, 4);
  //acende a luz de fundo
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Inicializando...");
  lcd.setCursor(0,1);
  lcd.print("Suporte: ");
  lcd.setCursor(0,2);
  lcd.print("recifemecatron.com");
  lcd.setCursor(0,3);
  lcd.print("(81)98797-6280");
  delay(3000);
  lcd.clear();

  printHelp();
}

void loop() {
  DateTime now = rtc.now();

  int dia_do_ano = dayOfYear(now.month(), now.day(), now.year());

  for (int i = 0; i < NUM_BOMBAS; i++) {
    bool deve_estar_ligado = ((dia_do_ano + i) % 2) == 0;
    
    if (deve_estar_ligado != status_bombas[i]) {
      digitalWrite(PINS_BOMBAS[i], deve_estar_ligado ? LOW : HIGH);
      status_bombas[i] = deve_estar_ligado;
    }
  }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  //lcd.print("Data: ");
  lcd.print(now.day());
  lcd.print("/");
  lcd.print(now.month());
  lcd.print("/");
  lcd.print(now.year());

    // Display the status of the pumps
  for (int i = 0; i < NUM_BOMBAS; i+=2) {
    lcd.setCursor(0, i/2 + 1);
    lcd.print("B");
    lcd.print(i+1);
    lcd.print(": ");
    lcd.print(status_bombas[i] ? "ON " : "OFF ");
    lcd.print("B");
    lcd.print(i+2);
    lcd.print(": ");
    lcd.print(status_bombas[i+1] ? "ON " : "OFF");
  }

  // Print debug information to the serial monitor
  /*
  Serial.println("Data: " + String(now.day()) + "/" + String(now.month()) + "/" + String(now.year()));
  for (int i = 0; i < NUM_BOMBAS; i++) {
    Serial.println("Bomba " + String(i+1) + ": " + (status_bombas[i] ? "ON" : "OFF"));
  }
  */

    if (Serial.available() > 0) {
    String command = Serial.readString(); // lê a string enviada
    //Serial2.println(command);
    processCommand(command);
  }

  delay(2000);
}

void printHelp() {
  Serial.println("CLI - Interface de Linha de Comando");
  Serial.println("Os seguintes comandos estão disponíveis:");
  Serial.println("  STATUS - Imprime o status de todas as bombas");
  Serial.println("  BOMBA<n> ON - Liga a bomba <n>");
  Serial.println("  BOMBA<n> OFF - Desliga a bomba <n>");
  Serial.println("  DATAHORA - Imprime a data e hora atuais");
  Serial.println("  AJUSTARDATAHORA <aaaa> <mm> <dd> <HH> <MM> <SS> - Ajusta a data e hora atuais");
  Serial.println("    <aaaa> - ano (quatro dígitos)");
  Serial.println("    <mm> - mês (dois dígitos)");
  Serial.println("    <dd> - dia (dois dígitos)");
  Serial.println("    <HH> - hora (dois dígitos, formato 24 horas)");
  Serial.println("    <MM> - minuto (dois dígitos)");
  Serial.println("    <SS> - segundo (dois dígitos)");
  Serial.println("");
}

void printDateTime(){
  DateTime now = rtc.now();
  Serial.print("Data: ");
  Serial.print(now.day());
  Serial.print("/");
  Serial.print(now.month());
  Serial.print("/");
  Serial.print(now.year());
  Serial.print(" Hora: ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.println(now.second());
}

void adjustDateTime(String dateTime) {
  // separa a data e a hora
  int day = dateTime.substring(0, 2).toInt();
  int month = dateTime.substring(3, 5).toInt();
  int year = dateTime.substring(6, 10).toInt();
  int hour = dateTime.substring(11, 13).toInt();
  int minute = dateTime.substring(14, 16).toInt();
  int second = dateTime.substring(17, 19).toInt();

  // ajusta a data e a hora
  rtc.adjust(DateTime(year, month, day, hour, minute, second));
  Serial.println("Data e hora ajustadas para: " + dateTime);
}

void printInDisplay(String message){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
}

void processCommand(String command) {
  command.trim();
  
  if (command == "STATUS") {
    for (int i = 0; i < NUM_BOMBAS; i++) {
      Serial.println("Bomba " + String(i+1) + ": " + (status_bombas[i] ? "ON" : "OFF"));
    }
  } else if (command.startsWith("BOMBA")) {
    int bomba = command.charAt(5) - '1';
    if (0 <= bomba && bomba < NUM_BOMBAS) {
      String action = command.substring(7);
      if (action == "ON") {
        status_bombas[bomba] = true;
        digitalWrite(PINS_BOMBAS[bomba], HIGH);
        Serial.println("Bomba " + String(bomba + 1) + " ligada");
      } else if (action == "OFF") {
        status_bombas[bomba] = false;
        digitalWrite(PINS_BOMBAS[bomba], LOW);
        Serial.println("Bomba " + String(bomba + 1) + " desligada");
      } else {
        Serial.println("Comando desconhecido");
      }
    } else {
      Serial.println("Número de bomba inválido");
    }
  } else if (command == "DATAHORA"){
    printDateTime();
  } else if (command.startsWith("DATE")) {
      adjustDateTime(command.substring(5)); // remove o "DATE,"
  } else if (command.startsWith("DISPLAY")) {
    printInDisplay(command.substring(8));
  } else if (command.startsWith("AJUDA")){
    printHelp();
  } else {
    Serial.println("Comando desconhecido");
  }
}