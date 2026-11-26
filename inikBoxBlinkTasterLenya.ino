#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_NeoPixel.h>

U8G2_SSD1306_64X48_ER_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE);

const int PIN_SCHW = 4;  // schwarz
const int PIN_ROT  = 5;  // rot
const int PIN_LED  = 7;  // RGB-LED

Adafruit_NeoPixel led(1, PIN_LED, NEO_GRB + NEO_KHZ800);

const unsigned long STARTINTERVALL = 300;  // Anfangsgeschwindigkeit (ms)
const unsigned long AENDERUNG      = 100;  // Aenderung je Tasterdruck (ms)
const unsigned long UNTERGRENZE    = 100;  // so schnell wie es geht
const unsigned long OBERGRENZE     = 2000; // langsames Ende -> dort ist LED dann"aus"

unsigned long previousMillis = 0;
unsigned long interval = STARTINTERVALL;  // startet mit der Anfangsgeschwindigkeit
bool ledState = false;          // leuchtet die LED gerade?
int  farbe = 0;                 // 0 rot, 1 gruen, 2 blau

bool blinkt = true;    // false = LED bleibt aus

bool schwarzLetzt = false; // Tasterzustand im vorherigen Durchlauf
bool rotLetzt     = false;
unsigned long letzterDruck = 0; // wann wurde der letzte Druck akzeptiert?

void zeigeFarbe() {
  if (!ledState) {
    led.clear();
  } else if (farbe == 0) {
    led.setPixelColor(0, led.Color(255, 0, 0));
  } else if (farbe == 1) {
    led.setPixelColor(0, led.Color(0, 255, 0));
  } else {
    led.setPixelColor(0, led.Color(0, 0, 255));
  }
  led.show();
}

void setup() {
  Wire.begin(8, 10);

  pinMode(PIN_SCHW, INPUT_PULLUP);
  pinMode(PIN_ROT,  INPUT_PULLUP);

  display.begin();
  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);
  display.setCursor(17, 29);
  display.print("Lenya");
  display.sendBuffer();

  led.begin();
  led.setBrightness(40);   // volle Helligkeit blendet
  led.clear();
  led.show();
}

void loop() {
  unsigned long currentMillis = millis();

  // ---- Taster lesen: gegen GND geschaltet, gedrueckt = LOW ----
  bool schwarz = (digitalRead(PIN_SCHW) == LOW);
  bool rot     = (digitalRead(PIN_ROT)  == LOW);

  if (currentMillis - letzterDruck > 200) {

    // schwarzer Taster: schneller blinken -> interval kleiner
    if (schwarz && !schwarzLetzt) {
      letzterDruck = currentMillis;
      if (!blinkt) {
        // Blinken wieder einschalten - und dabei wieder
        // auf die Anfangsgeschwindigkeit zuruecksetzen
        blinkt = true;
        interval = STARTINTERVALL;
        previousMillis = currentMillis;
      } else if (interval > UNTERGRENZE) {
        interval -= AENDERUNG;
        if (interval < UNTERGRENZE) {
          interval = UNTERGRENZE;   // Untergrenze einhalten
        }
      }
    }

    // roter Taster: langsamer blinken -> interval groesser
    if (rot && !rotLetzt) {
      letzterDruck = currentMillis;
      if (blinkt) {
        interval += AENDERUNG;
        if (interval >= OBERGRENZE) {
          // am langsamen Ende hoert das Blinken ganz auf
          interval = OBERGRENZE;
          blinkt   = false;
          ledState = false;
          zeigeFarbe();             // LED sofort ausschalten
        }
      }
    }
  }

  schwarzLetzt = schwarz;
  rotLetzt     = rot;

  if (blinkt && currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    ledState = !ledState;
    if (ledState) {
      farbe = (farbe + 1) % 3;
    }
    zeigeFarbe();
  }
}