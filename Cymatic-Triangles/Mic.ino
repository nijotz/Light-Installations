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

#define BRIGHTNESS_GRAVITY (2.0 / ANIMATE_SECS_PER_TICK)
#define BRIGHTNESS_INITIAL_VEL (-1.0 / ANIMATE_SECS_PER_TICK)
#define ROTATION_ACCEL (0.3 / ANIMATE_SECS_PER_TICK)
#define ROTATION_FRICTION (0.03 / ANIMATE_SECS_PER_TICK)

uint8_t dataPin = MIC_DATA_PIN;
CRGB mic_leds_rgb[NUM_MIC_LEDS];
double mic_leds_hue[NUM_MIC_LEDS];
double mic_leds_val[NUM_MIC_LEDS];
double mic_leds_vel[NUM_MIC_LEDS];


double current_led = (NUM_MIC_LEDS / 2);
double speed = NUM_MIC_LEDS;

void setupMic() {
  // Instantiate FastLED
  FastLED.addLeds<NEOPIXEL, MIC_DATA_PIN>(mic_leds_rgb, NUM_MIC_LEDS);

  // Start with full brightness LEDs that fade to test that code works
  for (int i = 0; i < NUM_MIC_LEDS; i++) {
    mic_leds_val[i] = 255.0;
    mic_leds_vel[i] = BRIGHTNESS_INITIAL_VEL;
  }
}

// Returns a number between 0 and 1 that represents current amplitude of sound
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

  // Use changes in amplitude to determine acceleration
  double peak = normalize((float)amp_sum_L);
  double rot_accel = peak - 0.5;
  if (rot_accel < 0) {
    rot_accel *= ROTATION_FRICTION;
  } else {
    rot_accel *= ROTATION_ACCEL;
  }

  // Apply acceleration to rotation speed
  speed = speed + rot_accel;
  if (speed < 0) { speed = 0; }
  if (speed > MAX_SPEED) { speed = MAX_SPEED; }

  // Rotate around the ring according to speed
  int last = (int)current_led;
  current_led += speed * ANIMATE_SECS_PER_TICK;
  current_led = fmod(current_led, (NUM_MIC_LEDS - 1));
  int curr = (int)current_led;

  // Max brightness and reset velocity of LEDs between the last and current
  while (last != curr) {
    mic_leds_val[last] = 255;
    mic_leds_vel[last] = BRIGHTNESS_INITIAL_VEL;
    last++;
    last %= (NUM_MIC_LEDS - 1);
  }

  // Assign hue based on current audio
  mic_leds_hue[curr] = color.hue;

  // Apply brightness velocities to brightness values
  for (int i = 0; i < NUM_MIC_LEDS; i++) {
    mic_leds_val[i] += mic_leds_vel[i] * ANIMATE_SECS_PER_TICK;
    if (mic_leds_val[i] < 0) { mic_leds_val[i] = 0; }
    if (mic_leds_val[i] > 255) { mic_leds_val[i] = 255; }
  }

  // Apply values to the FastLED array
  for (int i = 0; i < NUM_MIC_LEDS; i++) {
    mic_leds_rgb[i] = CHSV(mic_leds_hue[i], 255, mic_leds_val[i]);
  }
}
