#include <RCSwitch.h>
#include "FastLED.h"
#define  NUM_LEDS 77
#define DATA_PIN 6
#define CLOCK_PIN 7

CRGB leds[NUM_LEDS];
RCSwitch Switch = RCSwitch();
int a = 0;

void setup()
{
  Serial.begin(9600);
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN>(leds, NUM_LEDS);
  //FastLED.setBrightness(40);
  Switch.enableTransmit(10);   //Pin 10 am Arduino
  Switch.setRepeatTransmit(5); // Wiederholungen
}


void loop() {
  FastLED.clear();
  a = 0;
  stat();
  //logo();
  logomove();
  FastLED.show();
  Switch.send(a, 24);
  Serial.print(a);
  delay(3000);
}

void logo()
{
  for(int led = 0; led < 47; led++)
    {
      leds[led] = CRGB::Yellow;
    }
    FastLED.show();
}

void logomove()
{
  for(int led = 0; led < 47; led++)
    {
      leds[led] = CRGB::Yellow;
      FastLED.show();
      leds[led] = CRGB::Yellow;
      delay(50);
    }
}

void stat()
{
  int newCol = analogRead(14);
  Serial.println(newCol); 

 if (newCol < 750) // enspricht < 1,67V 341
  { a = 1;
    for(int led = 47; led < NUM_LEDS-20; led++) {
        leds[led] = CRGB::Green;
      }
    for(int led = 57; led < NUM_LEDS; led++) {
        leds[led] = CRGB::Black; 
    }
  }
  else if (newCol >= 1600) // entspricht >= 3,34 V 682
  { a = 2;

    for(int led = 47; led < NUM_LEDS-20; led++) {
        leds[led] = CRGB::Black; 
    }
    for(int led = 57; led < NUM_LEDS-10; led++) {
        leds[led] = CRGB::Blue;
    }
    for(int led = 67; led < NUM_LEDS; led++) {
        leds[led] = CRGB::Black;
    }
  }
  else // enspricht > 1,67V und < 3,34V
  { a = 3;
    for(int led = 47; led < NUM_LEDS-10; led++) {
        leds[led] = CRGB::Black;
    }
    for(int led = 67; led < NUM_LEDS; led++) {
        leds[led] = CRGB( 0, 89, 255);    }
  }
}
