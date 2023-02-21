#include "screens.h"
#include <EEPROM.h>
int estado = 0, ultimoEstado;
bool isBackup = true;
uint8_t fileSelected = 0;
unsigned long lastMillis = millis(), readMillis = millis(), case1Millis = millis();
void setup() {
  analogReference(INTERNAL);
  pinMode(12, INPUT);
  pinMode(A7, INPUT);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  beginSerial();
  beginScreen();
  beginTime();
  //adjustTime();
  readTime();
}
void loop() {
  switch (estado) {
    /////////////////////////////////////
    case 0:
      readButtons();
      readSerial();
      mainScreen();
      if (millis() - readMillis > 3333) {
        uint16_t aux, aux2;
        readTime();
        if (newHour()) {
          if (isBackup) {
            if (minHumedad > EEPROM.read(1000)) minHumedad = EEPROM.read(1000);
            if (maxHumedad < EEPROM.read(1001)) maxHumedad = EEPROM.read(1001);
            EEPROM.get(1002, aux);
            if (minTemperatura > aux) EEPROM.get(1002, minTemperatura);
            EEPROM.get(1004, aux2);
            if (maxTemperatura < aux2) EEPROM.get(1004, maxTemperatura);
            isBackup = false;
          }
          EEPROM.write(lastHour * 6 + 0, minHumedad);
          EEPROM.write(lastHour * 6 + 1, maxHumedad);
          EEPROM.put(lastHour * 6 + 2, minTemperatura);
          EEPROM.put(lastHour * 6 + 4, maxTemperatura);
          if (lastHour == 23) {
            for (int j = 0; j < 4; j++) {
              for (int i = 0; i < 24; i++) {
                uint8_t auxx = EEPROM.read(200 * (3 - j) + (i * 6) + 0);
                uint8_t auxx2 = EEPROM.read(200 * (3 - j) + (i * 6) + 1);
                EEPROM.write(200 * (4 - j) + (i * 6) + 0, auxx);
                EEPROM.write(200 * (4 - j) + (i * 6) + 1, auxx2);
                EEPROM.get(200 * (3 - j) + (i * 6) + 2, aux);
                EEPROM.put(200 * (4 - j) + (i * 6) + 2, aux);
                EEPROM.get(200 * (3 - j) + (i * 6) + 4, aux2);
                EEPROM.put(200 * (4 - j) + (i * 6) + 4, aux2);
              }
            }
          }
          maxHumedad = humedad;
          minHumedad = humedad;
          maxTemperatura = temperatura;
          minTemperatura = temperatura;
        }
        if (tenMinutes()) {
          EEPROM.write(1000 + 0, minHumedad);
          EEPROM.write(1000 + 1, maxHumedad);
          EEPROM.put(1000 + 2, minTemperatura);
          EEPROM.put(1000 + 4, maxTemperatura);
          isBackup = false;
        }
        readMillis = millis();
      }
      if (boton1) {
        estado = 2;
        lastMillis = millis();
      }
      if (intentos >= 60) {
        estado = 1;
        lastMillis = millis();
        ultimoEstado = 0;
      }
      resetButtons();
      noActivity();
      break;
    /////////////////////////////////////
    case 1:
      readButtons();
      if (millis() - case1Millis > 1000) {
        readSerial();
        noCableScreen();
        case1Millis = millis();
      }
      if (boton1) {
        estado = 2;
        lastMillis = millis();
      }
      if (intentos == 0) {
        estado = 0;
        lastMillis = millis();
        ultimoEstado = 1;
      }
      resetButtons();
      noActivity();
      break;
    //////////////////////////////////////
    case 2:
      readButtons();
      filesScreen(fileSelected);
      if (isIdle) {
        ultimoEstado = 2;
        estado = 1;
        fileSelected = 0;
      }
      if (boton1) {
        if (fileSelected) {
          estado = 3;
        }
        else {
          estado = 1;
          fileSelected = 0;
        }
      }
      if (boton2) {
        fileSelected++;
        if (fileSelected >= 5) {
          fileSelected = 0;
        }
      }
      if (digitalRead(5) == 0) {
        sendData(5);
      }
      resetButtons();
      noActivity();
      break;
    ////////////////////////////////////
    case 3:
      sendingScreen();
      sendData(fileSelected);
      doneScreen();
      delay(2000);
      fileSelected = 0;
      estado = 1;
      break;
      ///////////////////////////////////
  }
}
