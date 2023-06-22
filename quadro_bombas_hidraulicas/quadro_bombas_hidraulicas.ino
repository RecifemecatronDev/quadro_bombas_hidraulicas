#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>

#define NUM_BOMBAS 6

const int PINS_BOMBAS[NUM_BOMBAS] = {D3, D4, D5, D6, D7, D8};

RTC_DS1307 rtc;

bool status_bombas[NUM_BOMBAS];

LiquidCrystal_I2C lcd(0x27, 20, 4);

int dayOfYear(int month, int day, int year) {
    int monthDays[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    // Ajusta para anos bissextos
    if (month > 2 && year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        return monthDays[month - 1] + day + 1;
    }
    
    return monthDays[month - 1] + day;
}

void setup() {
  Wire.begin();
  rtc.begin();
  
  if (!rtc.isrunning()) {
    rtc.adjust(DateTime(2023, 6, 22, 12, 0, 0));
  }

  for (int i = 0; i < NUM_BOMBAS; i++) {
    pinMode(PINS_BOMBAS[i], OUTPUT);
    digitalWrite(PINS_BOMBAS[i], LOW);
    status_bombas[i] = false;
  }
  
  lcd.begin(20, 4);
  lcd.print("Inicializando...");
  delay(2000);
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();

  int dia_do_ano = dayOfYear(now.month(), now.day(), now.year());

  for (int i = 0; i < NUM_BOMBAS; i++) {
    bool deve_estar_ligado = ((dia_do_ano + i) % 2) == 0;
    
    if (deve_estar_ligado != status_bombas[i]) {
      digitalWrite(PINS_BOMBAS[i], deve_estar_ligado ? HIGH : LOW);
      status_bombas[i] = deve_estar_ligado;
    }
  }
  
  lcd.setCursor(0, 0);
  lcd.print("Data: ");
  lcd.print(now.day());
  lcd.print("/");
  lcd.print(now.month());
  lcd.print("/");
  lcd.print(now.year());
  
  for (int i = 0; i < NUM_BOMBAS; i++) {
    lcd.setCursor(0, i+1);
    lcd.print("Bomba ");
    lcd.print(i+1);
    lcd.print(": ");
    lcd.print(status_bombas[i] ? "ON" : "OFF");
  }
  
  delay(1000);
}
