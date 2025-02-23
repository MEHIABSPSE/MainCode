
/*
kód psaný 23.2.2025
Autor Dominik Hybler
*/
#include <SoftwareSerial.h>

//knihovny pro bmp280
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
//knihovna pro gps
#include <TinyGPS.h>
SoftwareSerial swSerial(5, 6);    //RX, TX gps
SoftwareSerial mySerial(10, 11);  // RX, TX komunikace
TinyGPS gps;
#define BMP280_ADRESA (0x76)  //adresa pro bmp280
Adafruit_BMP280 bmp;
int korekce = 32;
void setup() {
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
}

void loop() {  // run over and over
  //proměnné na odesílání
  float teplota = bmp.readTemperature();
  float tlak = (bmp.readPressure() / 100.00) + korekce;
  bool novaData = false;
  unsigned long znaky;
  unsigned short slova, chyby;
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
    Serial.print(" Rychlost v km/h: ");
    Serial.println(gps.f_speed_kmph() == TinyGPS::GPS_INVALID_F_SPEED ? 0 : gps.f_speed_kmph());
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
  Serial.println();
  //odesílání přes tx a rx
  mySerial.println(teplota);
  mySerial.println(tlak);
  delay(1000);
}
