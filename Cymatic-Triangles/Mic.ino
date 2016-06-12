#include "Wisp.h" /* Import Wisp class */
#include <FastLED.h>
#include "SPI.h" // Comment out this line if using Trinket or Gemma

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <math.h>

#define NUM_MIC_LEDS 28
#define MIC_DATA_PIN 8

#define MAX_ROTATIONS_PER_SEC 4.0
#define MAX_SPEED (NUM_MIC_LEDS * MAX_ROTATIONS_PER_SEC) /* LEDs per second */

#define BRIGHTNESS_GRAVITY 120.0
#define BRIGHTNESS_INITIAL_VEL -100.0
#define ROTATION_ACCEL_COEFF 1.0

uint8_t dataPin = MIC_DATA_PIN;
CRGB mic_leds_rgb[NUM_MIC_LEDS];
double mic_leds_hue[NUM_MIC_LEDS];
double mic_leds_val[NUM_MIC_LEDS];
double mic_leds_vel[NUM_MIC_LEDS];


double current_led = 5.0;
double speed = 30.0;

void setupMic() {
  /* Instantiate FastLED */
  FastLED.addLeds<NEOPIXEL, MIC_DATA_PIN>(mic_leds_rgb, NUM_MIC_LEDS);

  for (int i = 0; i < NUM_MIC_LEDS; i++) {
    mic_leds_val[i] = 255.0;
    mic_leds_vel[i] = BRIGHTNESS_INITIAL_VEL;
  }
}

float normalize(float x) {
  x = x - min_amplitude;
  x = x / (max_amplitude - min_amplitude);
  if (x > 1) { x = 1; }
  if (x < 0) { x = 0; }
  return x;
}

void animateMic() {
  // Apply brightness gravity to all LEDs
  for (int i = 0; i < NUM_MIC_LEDS; i++) {
    double vel = mic_leds_vel[i];
    vel += (-BRIGHTNESS_GRAVITY) * ANIMATE_SECS_PER_TICK;
    if (mic_leds_val[i] < 1 && vel < 0) { vel = 0; }
    mic_leds_vel[i] = vel;
  }

  // Rotate color values
  for (int i = 0; i < NUM_MIC_LEDS; i++) {
    double hue = mic_leds_hue[i];
    hue += 0.1;
    hue = fmod(hue, 255);
    mic_leds_hue[i] = hue;
  }

  double peak = normalize((float)amp_sum_L);
  double rot_accel = (peak - 0.5) * ROTATION_ACCEL_COEFF;

  speed = speed + rot_accel;
  if (speed < 0) { speed = 0; }
  if (speed > MAX_SPEED) { speed = MAX_SPEED; }

  // Rotate around the ring
  int last = (int)current_led;
  current_led += speed * ANIMATE_SECS_PER_TICK;
  current_led = fmod(current_led, (NUM_MIC_LEDS - 1));
  int curr = (int)current_led;

  // Max the brightness and reset the velocity of every LED between the last
  // and the current
  while (last != curr) {
    mic_leds_val[last] = 255;
    mic_leds_vel[last] = BRIGHTNESS_INITIAL_VEL;
    last++;
    last %= (NUM_MIC_LEDS - 1);
  }

  // Apply brightness velocities to brightness values
  for (int i = 0; i < NUM_MIC_LEDS; i++) {
    mic_leds_val[i] += mic_leds_vel[i] * ANIMATE_SECS_PER_TICK;
    if (mic_leds_val[i] < 0) { mic_leds_val[i] = 0; }
    if (mic_leds_val[i] > 255) { mic_leds_val[i] = 255; }
  }

  for (int i = 0; i < NUM_MIC_LEDS; i++) {
    mic_leds_rgb[i] = CHSV(mic_leds_hue[i], 255, mic_leds_val[i]);
  }
}
