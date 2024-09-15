#pragma region Includes
#include <Adafruit_NeoPixel.h>
#include <WiFiManager.h>
#include <NTPClient.h>
//#include <LibPrintf.h>

#pragma endregion Includes

#pragma region Einstellungen und Pins
//An welchem Pin ist es angeschlossen
#define SECONDPIN D3
#define MINUTEPIN D4
#define HOURPIN D8


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

#pragma endregion Einstellungen und Pins

#pragma region Globale Variablen
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

uint32_t currTime = 0;
uint32_t lastTimeUpdate = 0;
uint32_t lastCycleUpdate = 0;

Adafruit_NeoPixel secondstripe(PIXELCOUNTSECONDS, SECONDPIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel minutestripe(PIXELCOUNTMINUTES, MINUTEPIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel hourstripe(PIXELCOUNTHOURS, HOURPIN, NEO_RGB + NEO_KHZ800);

int hours = 0;
int minutes = 0;
int seconds = 0;
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

void setup() {
  pinMode(D1,OUTPUT); digitalWrite(D1,HIGH);
  secondstripe.begin();
  minutestripe.begin();
  hourstripe.begin();

  secondstripe.setBrightness(BRIGHTNESS);
  minutestripe.setBrightness(BRIGHTNESS);
  hourstripe.setBrightness(BRIGHTNESS);



  secondstripe.clear();
  minutestripe.clear();
  hourstripe.clear();
  Serial.begin(9600);
  setupWIFI();
  setupNTP();
}

void loop() {
  currTime = millis();
  if (currTime >= lastTimeUpdate + 10 * 60 * 1000) {
    if (timeClient.update()) {
      Serial.println("Got NTP Time!");
    } else {
      Serial.println("NTP not reachable");
    }
    lastTimeUpdate = currTime;
    minutes = timeClient.getMinutes();
    seconds = timeClient.getSeconds();
     hours = timeClient.getHours();
    lastCycleUpdate = currTime;

    Serial.println(timeClient.getFormattedTime());
  }
  
  if (currTime >= lastCycleUpdate + 1000) {
    seconds++;
    if (seconds == 60) {
      seconds = 0;
      minutes++;
    }
    if (minutes == 60) {
      minutes = 0;
      hours++;
    }
    if (hours == 24) {
      hours = 0;
    }

    displayBCDClock(hours, minutes, seconds);

    if (currTime >= lastCycleUpdate + 4000)
      lastCycleUpdate = currTime;
    else
      lastCycleUpdate += 1000;
  }
  
  uint32_t sleepDuration = (lastCycleUpdate + 1000) - currTime;
  if(sleepDuration > 10) {
    delay(sleepDuration - 10);
  }
}

void setupWIFI() { // Stellt eine Verbindung zum Wlan her
  WiFi.mode(WIFI_STA);

  WiFiManager wm;
  
  // TODO: die Einstellungen zurücksetzen, wenn hier beim Starten ein Knopf gedrückt wird
  //wm.resetSettings();

  // Normalerweise blockiert WiFiManager, bis wir verbunden sind, und nimmt uns so einiges an Code Arbeit ab.
  // Jedoch wollen wir eine Animation beim Booten anzeigen, und benutzen daher Blocking = false, müssen dafür
  // aber selbst wiederholt wm.process() ausführen, bis wir verbunden sind.
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

void setupNTP() {  // Fragt mit einer bereits offenen WLAN Verbindung das erste mal nach der Zeit
  timeClient.begin();
  timeClient.setTimeOffset(7200);
  timeClient.update();
  hours = timeClient.getHours();
  minutes = timeClient.getMinutes();
  seconds = timeClient.getSeconds();
  lastTimeUpdate = millis();
}

byte bcd_time=0;
void displayBCDClock(byte hour, byte minute, byte second) {
  bcd_time += 5;
  displayBCDSegment(hour, hourstripe, 16);
  displayBCDSegment(minute, minutestripe, 8);
  displayBCDSegment(second, secondstripe, 0);
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
