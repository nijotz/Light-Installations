#include "Wisp.h" /* Import Wisp class */
#include <FastLED.h>
#include "SPI.h" // Comment out this line if using Trinket or Gemma

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#define NUM_MIC_LEDS 27 /* LED's in the strip */
#define MIC_DATA_PIN 8 /* LED Data Pin */

#define WISP_1_HUE 0 /* Wisp 1 starting hue */
#define WISP_2_HUE 20 /* Wisp 2 starting hue */
#define WISP_1_START_POS 0 /* Wisp 1 starting position */
#define WISP_2_START_POS 15 /* Wisp 2 starting position */
#define WISP_HUE_INCREMENT 2 /* How much the color of the Wisps changes per loop */
#define BASE_TRAIL 13 /* Starting wisp trail length */
#define BASE_BRIGHTNESS_STEP 8 /* Starting brightness increment between wisp and trail elements */
#define BASE_HUE_STEP 3 /* Starting hue increment between wisp and trail elements */
#define BASE_BRIGHTNESS 100 /* Base Wisp brightness */
#define BRIGHTNESS_EASE_INCREMENT 5 /* How much Wisp brightness changes per step */

uint8_t dataPin = MIC_DATA_PIN;    // Yellow wire on Adafruit Pixels
CRGB mic_leds[NUM_MIC_LEDS]; /* Initialize FastLED leds array */

/* Instantiate Wisp objects (you can add more than 2) */
Wisp wisp1(WISP_1_START_POS, BASE_TRAIL, BASE_BRIGHTNESS, WISP_1_HUE);
Wisp wisp2(WISP_2_START_POS, BASE_TRAIL, BASE_BRIGHTNESS, WISP_2_HUE);

/* Variables to be changed in the loop */
int brightness_step = BASE_BRIGHTNESS_STEP; /* How much brightness is lost from one trail pixel to the next */
int new_brightness; /* Temp variable for the new calculated brightness */

void setupMic() {
  /* Instantiate FastLED */
  FastLED.addLeds<NEOPIXEL, MIC_DATA_PIN>(mic_leds, NUM_MIC_LEDS);
  FastLED.show();
}

float normalize(float x) {
  x = x - min_amplitude;
  x = x / max_amplitude;
  if (x > 1) { x = 1; }
  return x;
}

void animateMic() {
  float peak = normalize((float)amp_sum_L);

  /* Calculate new brightness, ease brightness toward the new value */
  new_brightness = BASE_BRIGHTNESS + (int)(peak * 100);
  if(wisp1.get_brightness() < new_brightness && wisp1.get_brightness() < (new_brightness - BRIGHTNESS_EASE_INCREMENT)) {
    wisp1.set_brightness(wisp1.get_brightness() + BRIGHTNESS_EASE_INCREMENT);
    wisp2.set_brightness(wisp2.get_brightness() + BRIGHTNESS_EASE_INCREMENT);
  } else if(wisp1.get_brightness() > new_brightness && wisp1.get_brightness() > (new_brightness + BRIGHTNESS_EASE_INCREMENT)) {
    wisp1.set_brightness(wisp1.get_brightness() - BRIGHTNESS_EASE_INCREMENT);
    wisp2.set_brightness(wisp2.get_brightness() - BRIGHTNESS_EASE_INCREMENT);
  } else {
    wisp1.set_brightness(new_brightness);
    wisp2.set_brightness(new_brightness);
  }

  /* Change the brightness step so taht brightness of trail pixels doens't go below 0 */
  brightness_step = new_brightness / (NUM_MIC_LEDS / 1.5);

  /* If Wisps reach the end of the strip, start over at the beginning */
  if(wisp1.get_pos() > (NUM_MIC_LEDS - 1)) wisp1.set_pos(0);
  if(wisp2.get_pos() > (NUM_MIC_LEDS - 1)) wisp2.set_pos(0);

  /* Shift Wisp hue over time, if it goes above 255, reset to 0 */
  wisp1.set_hue(wisp1.get_hue() + WISP_HUE_INCREMENT);
  wisp2.set_hue(wisp2.get_hue() + WISP_HUE_INCREMENT);
  if((wisp1.get_hue() + 2) > 255) wisp1.set_hue(0);
  if((wisp2.get_hue() + 2) > 255) wisp2.set_hue(0);

  /* Set the Wisp pixel color and brightness */
  mic_leds[wisp1.get_pos()] = CHSV(wisp1.get_hue(), 180, wisp1.get_brightness());
  mic_leds[wisp2.get_pos()] = CHSV(wisp2.get_hue(), 180, wisp2.get_brightness());

  /* Update Wisp positions and set trail pixel LED colors */
  wisp1.update(mic_leds, brightness_step, NUM_MIC_LEDS, BASE_HUE_STEP);
  wisp2.update(mic_leds, brightness_step, NUM_MIC_LEDS, BASE_HUE_STEP);
}

