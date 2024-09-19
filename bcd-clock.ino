#pragma region Includes
#include <Adafruit_NeoPixel.h>
#include <WiFiManager.h>
#include <time.h>
#pragma endregion Includes

#pragma region Einstellungen und Pins
// An welchem Pin ist es angeschlossen
#define SECONDPIN D3
#define MINUTEPIN D7
#define HOURPIN D8

#define ENABLE_SERIAL

// How many NeoPixels are attached to the Arduino?
#define PIXELCOUNTSECONDS 7
#define PIXELCOUNTMINUTES 7
#define PIXELCOUNTHOURS 6

#define BRIGHTNESS 10

#define IDLE_DELAY 101

//                                             -R-  -G-  -B-
#define HOUR_COLOR   (Adafruit_NeoPixel::Color(255,   0,   0))
#define MINUTE_COLOR (Adafruit_NeoPixel::Color(  0, 255,   0))
#define SECOND_COLOR (Adafruit_NeoPixel::Color(  0,   0, 255))

// Setze hier ANIM auf 1-5, die Definitionen sind hier drunter (du kannst auch deine eigenen Animationen rein machen)
#define ANIM 5

#define MY_NTP_SERVER "de.pool.ntp.org"
#define MY_TZ "CET-1CEST,M3.5.0,M10.5.0/3"
#pragma endregion Einstellungen und Pins

#pragma region Globale Variablen
Adafruit_NeoPixel secondstripe(PIXELCOUNTSECONDS, SECONDPIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel minutestripe(PIXELCOUNTMINUTES, MINUTEPIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel hourstripe(PIXELCOUNTHOURS, HOURPIN, NEO_RGB + NEO_KHZ800);
#pragma endregion Globale Variablen

#pragma region Definitionen der Boot-Animationen
  #define __ 255
  // Boot animation:
  // Farbverlauf Diagonal
  #pragma region
    #if ANIM == 1
    #define NUM_FRAMES 8
    #define ANIM_DELAY 5
    const byte animationData[] = {
      0, 1, 2, 3,
      1, 2, 3,__,
      2, 3, 4, 5,
      3, 4, 5,__,
      4, 5, 6, 7,
      5, 6,__,__,
    };
  #pragma endregion
  // Zick-Zack
  #pragma region
    #elif ANIM == 2
    #define NUM_FRAMES 32
    #define ANIM_DELAY 5
    const byte animationData[] = {
      0, 1, 2, 3,
      6, 5, 4,__,
      7, 8, 9,10,
      13,12,11,__,
      14,15,16,17,
      19,18,__,__,
    };
  #pragma endregion
  // Zufällig:
  #pragma region
    #elif ANIM == 3
    #define NUM_FRAMES 64
    #define ANIM_DELAY 8
    const byte animationData[] = {44,13,28,10,5,35,55,52,3,53,31,48,61,51,7,2,60,56,39,62,36,34,23,51};
  #pragma endregion
  // Ladekreis
  #pragma region
    #elif ANIM == 4
    #define NUM_FRAMES 16
    #define ANIM_DELAY 12
    const byte animationData[] = {
      __,__,__,__,
      6, 8,10,__,
      5,__,12,__,
      4,__,13,__,
      2, 0,14,__,
      __,__,__,__,
    };
  #pragma endregion
  // Innen nach Außen
  #pragma region
    #elif ANIM == 5
    #define NUM_FRAMES 16
    #define ANIM_DELAY 20
    const byte animationData[] = {
      3, 2, 2, 3,
      2, 1, 1, 2,
      1, 0, 0, 1,
      1, 0, 0, 1,
      2, 1, 1, 2,
      3, 2, 2, 3,
    };
    /*#define MASK_ANIM
    const byte maskData[] = {
      64,  64,  64, 64,
      64, 128, 128, 64,
    128, 255, 255, 128,
    128, 255, 255, 128,
      64, 128, 128, 64,
      64,  64,  64, 64,
    };*/
  #pragma endregion
  #endif
  #undef __

  #ifndef MASK_ANIM
  const byte maskData[] = { 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255 };
  #endif

  #pragma region Implementierung Animationen
  static_assert(NUM_FRAMES == 2 || NUM_FRAMES == 4 || NUM_FRAMES == 8 || NUM_FRAMES == 16 || NUM_FRAMES == 32 || NUM_FRAMES == 64 || NUM_FRAMES == 128, "NUM_FRAMES must be 2, 4, 8, 16, 32, 64 or 128");

  auto getIntensity(byte t, byte anim) {
    return (byte)((unsigned short) anim * (unsigned short)(256 / NUM_FRAMES) - (unsigned short) t);
  }

  auto getColor(byte t, byte anim, byte mask) {
    if (anim == 255) return secondstripe.Color(0,0,0);
    #ifdef MASK_ANIM
    return secondstripe.ColorHSV(256 * getIntensity(t, anim), 255, mask);
    #else
    return secondstripe.ColorHSV(256 * getIntensity(t, anim));
    #endif
  }

  void connectionAnimation(byte time) {  //während auf die Wifi Verbindung gewartet wird blinkt die Anzeige
    for (int i = 0; i < PIXELCOUNTSECONDS; i++) {
      auto c = getColor((byte) time, animationData[i+0], maskData[i+0]);
      secondstripe.setPixelColor(i, c);
    }
    for (int i = 0; i < PIXELCOUNTMINUTES; i++) {
      auto c = getColor((byte) time, animationData[i + 8], maskData[i + 8]);
      minutestripe.setPixelColor(i, c);
    }
    for (int i = 0; i < PIXELCOUNTHOURS; i++) {
      auto c = getColor((byte) time, animationData[i + 16], maskData[i + 16]);
      hourstripe.setPixelColor(i, c);
    }
    secondstripe.show();
    minutestripe.show();
    hourstripe.show();
  }

  #pragma endregion Implementierung Animationen
#pragma endregion Definitionen der Boot-Animationen

struct PersistentStorage {
  uint16_t magic;
  struct timeval tv;
};

uint16_t getYear() {
  time_t now;
  tm tm;
  time(&now);
  localtime_r(&now, &tm);

  return 1900 + tm.tm_year;
}

void setup() {
#ifdef ENABLE_SERIAL
  Serial.begin(9600);
#endif

  PersistentStorage stored;
  ESP.rtcUserMemoryRead(32, (uint32_t *)&stored, sizeof(stored));
  setTZ(MY_TZ);
  settimeofday(&stored.tv, NULL);

  uint16_t year = getYear();
  if (stored.magic != 0xbeef || year < 2000 || year > 2500) {
#ifdef ENABLE_SERIAL
    Serial.print("Cold reboot detected, waiting for NTP");
#endif
    setupWIFI();
    configTime(MY_TZ, MY_NTP_SERVER);

    do {
      delay(100);
      year = getYear();
#ifdef ENABLE_SERIAL
      Serial.print(".");
#endif
    } while(year < 2000 || year > 2500);

#ifdef ENABLE_SERIAL
    Serial.println();
#endif
  }

  pinMode(D1, OUTPUT);
  digitalWrite(D1, HIGH);
  secondstripe.begin();
  minutestripe.begin();
  hourstripe.begin();
  secondstripe.setBrightness(BRIGHTNESS);
  minutestripe.setBrightness(BRIGHTNESS);
  hourstripe.setBrightness(BRIGHTNESS);

  time_t now;
  tm tm;
  time(&now);
  localtime_r(&now, &tm);
  displayBCDClock(tm.tm_hour, tm.tm_min, tm.tm_sec);



  stored.magic = 0xbeef;
  gettimeofday(&stored.tv, NULL);
  stored.tv.tv_sec++;
  uint32_t sleep = 1000 * 1000 - stored.tv.tv_usec;
  stored.tv.tv_usec = 0;
  ESP.rtcUserMemoryWrite(32, (uint32_t *)&stored, sizeof(stored));

#ifdef ENABLE_SERIAL
  Serial.print("going to sleep for (us) ");
  Serial.println(sleep);
#endif
  ESP.deepSleep(sleep, WAKE_RF_DISABLED);
}

void loop() {
  // this should never be reached
  delay(1000);
}

void setupWIFI() { // Stellt eine Verbindung zum Wlan her
  WiFi.mode(WIFI_STA);

  WiFiManager wm;
  wm.setConfigPortalBlocking(false);
  wm.autoConnect("BCD-UhrM");

  byte frame = 0;
  while (WiFi.status() != WL_CONNECTED) {
    connectionAnimation(frame);
    delay(ANIM_DELAY);
    frame++;
    wm.process();
  }
}

byte bcd_time = 0;
void displayBCDClock(byte hour, byte minute, byte second) {
  displayBCDSegment(hour, hourstripe, 16);
  displayBCDSegment(minute, minutestripe, 8);
  displayBCDSegment(second, secondstripe, 0);
  bcd_time += 5;
}

void displayBCDSegment(byte value, Adafruit_NeoPixel& display, byte animOffset) {
  display.clear();
  displayBCDLine(value % 10, display, 0, animOffset);
  displayBCDLine(value / 10, display, 4, animOffset);
  display.show();
}

void displayBCDLine(byte value, Adafruit_NeoPixel& display, byte stripeOffset, byte animOffset) {
    for(int i = 0; i < 4; i++) {
      if (value & (1 << i)) {
        auto c = getColor((byte) bcd_time, animationData[i + animOffset], maskData[i + animOffset]);
        display.setPixelColor(i + stripeOffset, c);
      }
    }
}
