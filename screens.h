#ifndef SCREENS_H
#define SCREENS_H
extern uint8_t subestado;
extern int intentos;
extern bool boton1, boton2, isIdle;
extern unsigned long lastMillis;
extern uint16_t maxTemperatura, minTemperatura, temperatura;
extern uint8_t minHumedad, maxHumedad, hora, minuto, lastHour, lastMinute, humedad;
void mainScreen();
void beginScreen();
void idleScreen(bool a);
void beginSerial();
void beginTime();
void readSerial();
void readTime();
void adjustTime();
void readButtons();
void resetButtons();
void noCableScreen();
void noActivity();
void filesScreen(uint8_t a);
void sendingScreen();
void sendData(uint8_t fileSelected);
void doneScreen();
uint8_t readI2C(uint8_t address);
uint8_t bcd2int(const uint8_t bcd);
uint8_t int2bcd(const uint8_t dec);
bool newHour();
bool tenMinutes();
uint8_t batteryCheck();
#endif
