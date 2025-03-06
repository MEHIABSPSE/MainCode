
/*
kód psaný 27.2.2025
Autor Dominik Hybler
*/
#include <SoftwareSerial.h>

//knihovny pro bmp280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
//knihovna pro gps
#include <TinyGPS.h>
//knihovna pro spektroskop
//#include <Adafruit_AS7341.h>
SoftwareSerial swSerial(5, 6);    //RX, TX gps
SoftwareSerial mySerial(10, 11);  // RX, TX komunikace
TinyGPS gps;
#define BMP280_ADRESA (0x76)  //adresa pro bmp280
Adafruit_BMP280 bmp;
//Adafruit_AS7341 as7341;
int korekce = 32;
void setup() {
  pinMode(4, OUTPUT);
  swSerial.begin(9600);
  Serial.begin(9600);
  mySerial.begin(9600);
  mySerial.println("Hello, world?");  //Test komunikace
                                      //pokud nebude bmp280 nalezeno, dojde k chybě
  if (!bmp.begin(BMP280_ADRESA)) {
    Serial.println("BMP280 senzor nenalezen, zkontrolujte zapojeni!");
    while (1)
      ;
  }
  while (!Serial) {
    delay(1);
  }
/*
  if (!as7341.begin()) {
    Serial.println("Could not find AS7341");
    while (1) { delay(10); }
  }
  

  as7341.setATIME(100);
  as7341.setASTEP(999);
  as7341.setGain(AS7341_GAIN_256X);
  */
}

void loop() {  // run over and over
  //proměnné na odesílání
    uint16_t readings[8];
  float teplota = bmp.readTemperature();
  float tlak = (bmp.readPressure() / 100.00) + korekce;
  bool novaData = false;
  unsigned long znaky;
  unsigned short slova, chyby;
  //spektroskop kód
  /*
   if (!as7341.readAllChannels(readings)) {
    Serial.println("Error reading all channels!");
    return;
  }
  Serial.print("ADC0/F1 415nm : ");
  Serial.println(readings[0]);
  Serial.print("ADC1/F2 445nm : ");
  Serial.println(readings[1]);
  Serial.print("ADC2/F3 480nm : ");
  Serial.println(readings[2]);
  Serial.print("ADC3/F4 515nm : ");
  Serial.println(readings[3]);
  Serial.print("ADC0/F5 555nm : ");
  Serial.println(readings[6]);
  Serial.print("ADC1/F6 590nm : ");
  Serial.println(readings[7]);
  Serial.print("ADC2/F7 630nm : ");
  Serial.println(readings[8]);
  Serial.print("ADC3/F8 680nm : ");
  Serial.println(readings[9]);
  */
  //gps kód
  for (unsigned long start = millis(); millis() - start < 1000;) {
    while (swSerial.available()) {
      char c = swSerial.read();
      if (gps.encode(c)) {
        novaData = true;
      }
    }
  }
  if (novaData) {
    float zSirka, zDelka;
    unsigned long stariDat;
    int rok;
    byte mesic, den, hodina, minuta, sekunda, setinaSekundy;
    // načtení GPS pozice do proměnných
    gps.f_get_position(&zSirka, &zDelka, &stariDat);
    // vytištění informací po sériové lince
    Serial.println("::Dostupne GPS udaje::");
    Serial.print("Zemepisna sirka: ");
    // nejprve zkontrolujeme, jestli máme platné údaje
    // (zSirka == TinyGPS::GPS_INVALID_F_ANGLE),
    // pokud nejsou validní (platné), vytiskneme nulu,
    // v opačném případě vytiskneme obsah proměnné s přesností 6 desetinných míst,
    // podobným způsobem se pracuje i s ostatními údaji
    Serial.print(zSirka == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : zSirka, 6);
    Serial.print(" delka: ");
    Serial.print(zDelka == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : zDelka, 6);
    Serial.print(" Pocet satelitu: ");
    Serial.println(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    Serial.print("Presnost: ");
    Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
    Serial.print(" Stari dat: ");
    Serial.print(stariDat == TinyGPS::GPS_INVALID_AGE ? 0 : stariDat);
    Serial.print(" Nadmorska vyska: ");
    Serial.print(gps.f_altitude() == TinyGPS::GPS_INVALID_F_ALTITUDE ? 0 : gps.f_altitude());
    // načtení data a času z GPS modulu do proměnných
    gps.crack_datetime(&rok, &mesic, &den, &hodina, &minuta, &sekunda, &setinaSekundy, &stariDat);
    // kontrola platnosti dat
    if (stariDat == TinyGPS::GPS_INVALID_AGE) {
      Serial.println("Nelze nacist datum a cas.");
    } else {
      // vytvoření proměnné pro vytištění data a času
      char datumCas[32];
      Serial.print("Datum a cas: ");
      // poskládání celé zprávy do proměnné datumCas a poté její vytištění,
      // %02d znamená desetinné číslo uvedené za uvozovkami s přesností na 2 číslice
      sprintf(datumCas, "%02d/%02d/%02d %02d:%02d:%02d", mesic, den, rok, hodina, minuta, sekunda);
      Serial.println(datumCas);
    }
  }
  // načtení a vytištění informací o komunikaci s GPS modulem
  gps.stats(&znaky, &slova, &chyby);
  Serial.print("Detekovane znaky: ");
  Serial.print(znaky);
  Serial.print(", slova: ");
  Serial.print(slova);
  Serial.print(", chyby pri kontrole dat: ");
  Serial.println(chyby);
  // kontrola chyb při komunikaci skrze detekci přijatých znaků
  if (znaky == 0) {
    Serial.println("Chyba pri prijmu dat z GPS, zkontrolujte zapojeni!");
  }
  if (gps.altitude()<5) {
  digitalWrite(4, 1);
  }

  Serial.println();
  //odesílání přes tx a rx
  float zSirka, zDelka;
  mySerial.print(zSirka == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : zSirka, 6);
mySerial.print(zDelka == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : zDelka, 6);
   mySerial.print(gps.f_altitude() == TinyGPS::GPS_INVALID_F_ALTITUDE ? 0 : gps.f_altitude());
  mySerial.println(teplota);
  mySerial.println(tlak);
  /*
  mySerial.print("ADC0/F1 415nm : ");
  mySerial.println(readings[0]);
  mySerial.print("ADC1/F2 445nm : ");
  mySerial.println(readings[1]);
  mySerial.print("ADC2/F3 480nm : ");
  mySerial.println(readings[2]);
  mySerial.print("ADC3/F4 515nm : ");
  mySerial.println(readings[3]);
  mySerial.print("ADC0/F5 555nm : ");
  mySerial.println(readings[6]);
  mySerial.print("ADC1/F6 590nm : ");
  mySerial.println(readings[7]);
  mySerial.print("ADC2/F7 630nm : ");
  mySerial.println(readings[8]);
  mySerial.print("ADC3/F8 680nm : ");
  mySerial.println(readings[9]);
  */
  delay(1000);
}
