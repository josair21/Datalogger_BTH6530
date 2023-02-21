#include <SoftwareI2C.h>
#include <MCP7940.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <EEPROM.h>
#include <LowPower.h>
#include "screens.h"
#define SDAI2C 10
#define SCLI2C 9
/*
  Font to use
  HUMIDITY: u8g2_font_luBIS12_tn
  TEMPERATURE: u8g2_font_logisoso32_tn
  GENERAL TEXT: u8g2_font_lubR08_tr
  LIST DATA: u8g2_font_7x14_tr
  BIG ADVICES: u8g2_font_crox5tb_tr
  BIG ICONS: u8g2_font_open_iconic_www_6x_t (modified)
  BATTERY: u8g2_font_battery24_tr
*/
uint8_t subestado = 0;
int intentos = 0;
bool isIdle, boton1, boton2, firstTime = true;
char datePrint[64];
uint16_t maxTemperatura, minTemperatura;
uint8_t minHumedad, maxHumedad;
//char dias[7][7] = {"Domin.", "Lunes", "Martes", "Mierc.", "Jueves", "Viern.", "Sabado"};
uint8_t hora = 0, minuto = 0, lastHour, lastMinute;
uint8_t humedad;
uint16_t temperatura;
//MCP7940_Class MCP;
SoftwareI2C MCP7940;
DateTime now;
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
//U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* reset=*/ U8X8_PIN_NONE);
void beginScreen() {
  u8g2.begin();
}
void mainScreen() {
  u8g2.firstPage();
  do {
    //BATERIA
    u8g2.setFont(u8g2_font_battery24_tr);
    u8g2.drawGlyph(108, 27, batteryCheck());
    //TRANSFER ICON
    u8g2.setFont(u8g2_font_open_iconic_www_6x_t);
    u8g2.drawGlyph(0, 64, 67); //Transfer icon
    //DATE
    u8g2.setFont(u8g2_font_lubR08_tr);
    u8g2.setCursor(2, 13);
    u8g2.print(datePrint);
    //HUMIDITY
    u8g2.setFont(u8g2_font_luBIS12_tn);
    u8g2.setCursor(48 + 1 + 18, 29);
    char aux[4]; sprintf(aux, "%2d", humedad);
    u8g2.print(aux);
    u8g2.setFont(u8g2_font_lubR08_tr);
    u8g2.setCursor(94, 27);
    u8g2.print("%");
    //Temperatura
    u8g2.setFont(u8g2_font_logisoso32_tn);
    u8g2.setCursor(48 + 1, 64);
    //float aux2 = temperatura / 10.0;
    //Serial.println(temperatura);
    //Serial.println(aux2);
    sprintf(aux, "%2d.%01d", temperatura / 10, temperatura % 10);
    u8g2.print(aux);
    u8g2.setCursor(120, 64 - 2);
    u8g2.setFont(u8g2_font_lubR08_tr);
    u8g2.setCursor(122, 38);
    u8g2.print("o");
  } while (u8g2.nextPage());
}
void idleScreen(bool a) {
  u8g2.setContrast(!a * 255);
  isIdle = a;
}
void beginSerial() {
  Serial.begin(9600);
}
void readSerial() {
  char dataArray[64];
  int dataLength = Serial.available();
  if (dataLength > 0) {
    for (int i = 0; i < dataLength; i++) {
      dataArray[i] = Serial.read();
    }
    if (dataArray[0] == 'P') {
      uint8_t humedadAux  = (dataArray[13] - '0') * 10 +  dataArray[14] - '0';
      uint16_t temperaturaAux = (dataArray[21] - '0') * 100 + ( dataArray[22] - '0') * 10 + dataArray[24] - '0';
      if (humedad - 5 < humedadAux && humedadAux < humedad + 5 && temperatura - 50 < temperaturaAux && temperaturaAux < temperatura + 50 && !firstTime) {
        humedad = humedadAux;
        temperatura = temperaturaAux;
        if (humedad > maxHumedad) maxHumedad = humedad;
        if (temperatura > maxTemperatura) maxTemperatura = temperatura;
        if (minHumedad > humedad) minHumedad = humedad;
        if (minTemperatura > temperatura) minTemperatura = temperatura;
      }
      else if (10 < humedadAux && humedadAux < 96 && 10 < temperaturaAux && temperaturaAux < 550 && firstTime) {
        humedad = humedadAux;
        temperatura = temperaturaAux;
        maxHumedad = humedad;
        minHumedad = humedad;
        maxTemperatura = temperatura;
        minTemperatura = temperatura;
        firstTime = false;
      }
      intentos = 0;
      /*Serial.println(maxHumedad);
        Serial.println(minHumedad);
        Serial.println(maxTemperatura);
        Serial.println(minTemperatura);*/
    }

  }
  else {
    intentos++;
    if (intentos > 1000) {
      intentos = 1000;
    }
  }
}
void beginTime() {
  MCP7940.begin(SCLI2C, SDAI2C);
}
void readTime() {
  bool isReady = false;
  //uint8_t segundo = bcd2int(readI2C(0x00) & 0x7F);
  if (digitalRead(SDAI2C) && digitalRead(SCLI2C)) {
    unsigned long auxMillis = millis();
    while (millis() - auxMillis < 2) {
      if (digitalRead(SDAI2C) == 0 | digitalRead(SCLI2C) == 0) return;
    }
  }
  else return;
  lastMinute = minuto;
  lastHour = hora;
  minuto = bcd2int(readI2C(0x01) & 0x7F);
  hora = bcd2int(readI2C(0x02) & 0x3F);
  //uint8_t dia = bcd2int(readI2C(0x04) & 0x3F);
  //uint8_t mes = bcd2int(readI2C(0x05) & 0x1F);
  //uint8_t anho = bcd2int(readI2C(0x06)) + 2000;
  if (0 <= minuto && minuto < 60 && 0 <= hora && hora < 24) {
    sprintf(datePrint, "%s   %02d: %02d", "Recording", hora, minuto);
  }
  //sprintf(datePrint, "Lunes 12, 15:34");
}
void adjustTime() {
  //MCP7940.adjust();
}
uint8_t readI2C(uint8_t address) {
  pinMode(SCLI2C, OUTPUT);
  pinMode(SDAI2C, OUTPUT);
  pinMode(SCLI2C, HIGH);
  pinMode(SDAI2C, HIGH);
  MCP7940.beginTransmission(0x6F);
  MCP7940.write(address);
  MCP7940.endTransmission();
  MCP7940.requestFrom(0x6F, 1);
  uint8_t aux = MCP7940.read();
  pinMode(SCLI2C, INPUT);
  pinMode(SDAI2C, INPUT);
  return aux;
}
void readButtons() {
  if (digitalRead(3) == LOW) {
    delay(300);
    while (digitalRead(3) == LOW);
    if (isIdle) lastMillis = millis();
    else {
      boton1 = true;
      lastMillis = millis();
    }
    return;
  }
  if (digitalRead(4) == LOW) {
    delay(300);
    while (digitalRead(4) == LOW);
    if (isIdle) lastMillis = millis();
    else {
      boton2 = true;
      lastMillis = millis();
    }
    return;
  }
}
void resetButtons() {
  boton1 = false;
  boton2 = false;
}
void noCableScreen() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_open_iconic_www_6x_t);
    u8g2.drawGlyph(74, 64, 69);
    u8g2.setFont(u8g2_font_crox5tb_tr);
    u8g2.drawStr(0, 20, "Check");
    u8g2.drawStr(0, 42, "Com.");
    u8g2.drawStr(0, 64, "Cable");
  } while ( u8g2.nextPage() );
}
void noActivity() {
  if (millis() - lastMillis > 30000) {
    idleScreen(true);
  }
  else idleScreen(false);
}
void filesScreen(uint8_t a) {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_logisoso16_tr);
    u8g2.drawStr(0, 15, "List of files:");
    u8g2.setFont(u8g2_font_lubR08_tr);
    u8g2.drawStr(30, 27, "DAY_1");
    u8g2.drawStr(30, 39, "DAY_2");
    u8g2.drawStr(30, 51, "DAY_3");
    u8g2.drawStr(30, 63, "DAY_4");
    if (a != 0) {
      u8g2.drawStr(5, 14 + 12 * a, "->");
    }
  } while ( u8g2.nextPage() );
}
void sendingScreen() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_open_iconic_www_6x_t);
    u8g2.drawGlyph(74, 64, 83);
    u8g2.setFont(u8g2_font_crox5tb_tr);
    u8g2.drawStr(0, 20, "Sendin'");
    u8g2.drawStr(0, 42, "All");
    u8g2.drawStr(0, 64, "Data");
  } while ( u8g2.nextPage() );
}
void sendData(uint8_t fileSelected) {
  char tempData[30];
  uint8_t aux, aux2;
  uint16_t aux3, aux4;
  Serial.println("item,minH,maxH,minT,maxT");
  for (int i = 0; i < 24; i++) {
    aux = EEPROM.read(200 * fileSelected + (i * 6) + 0);
    aux2 = EEPROM.read(200 * fileSelected + (i * 6) + 1);
    EEPROM.get(200 * fileSelected + (i * 6) + 2, aux3);
    EEPROM.get(200 * fileSelected + (i * 6) + 4, aux4);
    sprintf(tempData, "%d,%2d,%2d,%2d.%1d,%2d.%1d", i + 1, aux, aux2, aux3 / 10, aux3 % 10, aux4 / 10, aux4 % 10);
    Serial.println(tempData);
  }
}
void doneScreen() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_open_iconic_www_6x_t);
    u8g2.drawGlyph(74, 64, 73);
    u8g2.setFont(u8g2_font_crox5tb_tr);
    u8g2.drawStr(0, 20, "Brought");
    u8g2.drawStr(0, 42, "To");
    u8g2.drawStr(0, 64, "Pass");
  } while ( u8g2.nextPage() );
}
bool newHour() {
  if (lastHour != hora) {
    return true;
  }
  else return false;
}
bool tenMinutes() {
  if (lastMinute / 10 != minuto / 10) {
    return true;
  }
  else return false;
}
uint8_t bcd2int(const uint8_t bcd)  {
  return ((bcd / 16 * 10) + (bcd % 16));
}
uint8_t int2bcd(const uint8_t dec) {
  return ((dec / 10 * 16) + (dec % 10));
}
uint8_t batteryCheck() {
  //4.7k 1K
  int promedio = 0;
  /*if (!digitalRead(12)) {
    for (int i = 0; i < 5; i++) {
      promedio += analogRead(13);
    }
    }
    else return 63;*/
  for (int i = 0; i < 5; i++) {
    promedio += analogRead(A7);
  }
  promedio = promedio / 5;
  //Serial.println(promedio);
  if (700 < promedio) return 63;
  if (650 < promedio) return 60;
  if (600 < promedio) return 58;
  if (550 < promedio) return 56;
  if (500 < promedio) return 54;
  if (450 < promedio) return 48;
}
