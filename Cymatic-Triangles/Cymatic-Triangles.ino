#include <FastLED.h>
#include <math.h>
#include "EEPROM.h"

#define NUM_LEDS 57 // Number of LED's in the strip
#define DATA_PIN 6 // Data out
#define ANALOG_PIN_L 1 // Left audio channel
#define ANALOG_PIN_R 0 // Right audio channel
#define REFRESH_POT_PIN 2 // Left pot
#define SENSITIVITY_POT_PIN 3 // Right pot
#define STOMP_PIN 5 // The pin connected to the stomp button
#define STROBE_PIN 12 // Strobe pin
#define RESET_PIN 13 // Reset Pin

#define ANIMATE_TICKS 60.0
#define ANIMATE_SECS_PER_TICK (1.0 / ANIMATE_TICKS)
#define ANIMATE_MILLIS_PER_TICK (1000.0 / ANIMATE_TICKS)
#define AUDIO_TICKS 60
#define AUDIO_MILLIS_PER_TICK (1000 / AUDIO_TICKS)
#define AUDIO_TIME_SECONDS 1 // How much time it takes audio to travel to the outer edge of the display
#define SOUND_WAVE_LENGTH (AUDIO_TIME_SECONDS * AUDIO_TICKS)
#define SOUND_BUFFER_LENGTH 3 // How many audio snapshots to use for smoothing

#define SENSITIVITY_DIVISOR 100. // Higher = range of sensitivity values on pot is lower
#define LEFT_START_POINT ((NUM_LEDS / 2)) // Starting LED for left side
#define LEFT_END_POINT 1 // Generally the end of the left side is the first LED
#define RIGHT_START_POINT ((NUM_LEDS / 2) + 1) // Starting LED for the right side
#define RIGHT_END_POINT (NUM_LEDS - 1) // Generally the end of the right side is the last LED
#define LED_STACK_SIZE (NUM_LEDS) // How many LED's in each stack
#define MAX_AMPLITUDE 4700 // Maximum possible amplitude value
#define MAX_AMPLITUDE_MULTIPLIER 380
#define MIN_AMPLITUDE 545 // Lowest possible amplitude value (Higher number causes there to be more blank LED's)
#define MIN_AMPLITUDE_MULTIPLIER 200
#define SENSITIVITY_MULTIPLIER 200 // Higher = range of sensitivity values on pot is lower

int monomode; // Used to duplicate the left single for manual input
int next_audio_tick = 0; // Refresh rate of the animation
int next_animate_tick = 0; // Refresh rate of the animation
float min_amplitude = 500;
float max_amplitude = 4700;
int start_hue = 0;
int amp_sum_L = 0;
int amp_sum_R = 0;

float SOUND_LED_SCALE = float(SOUND_WAVE_LENGTH) / float(NUM_LEDS);

CRGB leds_inner_values[NUM_LEDS] = {0};
CRGB leds_outer_values[NUM_LEDS] = {0};

CRGB sound_wave[SOUND_WAVE_LENGTH] = {0};
int sound_buffer[SOUND_BUFFER_LENGTH] = {0};

// Set color value to full saturation and value. Set the hue to 0
CHSV color(0, 255, 255);
float leds_inner_mapping[NUM_LEDS]; // Represents LED strip
float leds_outer_mapping[NUM_LEDS]; // Represents LED strip

//____________Function declarations______
int get_freq_sum(int pin);
CRGB get_LED_color(int position, CRGB leds[], int value);
void change_color_mode();
int stomp_pressed();
void push_color_stack(CRGB stack[], int value);
void push_audio_stack(int stack[], int value);
void getAudiomsg();
void setsensitivity();
void animateTriangles(CRGB values[], float mappings[]);

void updateSoundWave();
CRGB interpolate_sound(CRGB sounds[], int index, int range);
void setupInnerTriangleMapping();
void setupOuterTriangleMapping();

void setupMic();
void animateMic();
//_______________________________________



void setup() {
  Serial.begin(9600);

  // Instantiate Neopixels with FastLED
  FastLED.addLeds<NEOPIXEL, DATA_PIN+1>(leds_inner_values, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds_outer_values, NUM_LEDS);

  // Clear any old values on EEPROM
  if (EEPROM.read(1) > 1){EEPROM.write(1,0);} // Clear EEPROM

  // Set pin modes
  pinMode(ANALOG_PIN_L, INPUT);
  pinMode(ANALOG_PIN_R, INPUT);
  pinMode(STOMP_PIN, INPUT);
  pinMode(STROBE_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);

  digitalWrite(RESET_PIN, LOW);
  digitalWrite(STROBE_PIN, HIGH);

  // If stomp is being pressed during setup, set monomode to True
  if (digitalRead(STOMP_PIN) == HIGH) {
    if (EEPROM.read(1) == 0) {
      EEPROM.write(1,1);
    } else if (EEPROM.read(1) == 1) {
      EEPROM.write(1,0);
    }
  }

  // Set monomode based on the EEPROM state
  monomode = EEPROM.read(1);

  setupInnerTriangleMapping();
  setupOuterTriangleMapping();

  setupMic();
}

void loop() {
  int current_time = millis();

  if (current_time > next_audio_tick) {
    next_audio_tick = current_time + AUDIO_MILLIS_PER_TICK;
    updateSoundWave();
  }

  if (current_time > next_animate_tick) {
    next_animate_tick = current_time + ANIMATE_MILLIS_PER_TICK;
    animateTriangles(leds_inner_values, leds_inner_mapping);
    animateTriangles(leds_outer_values, leds_outer_mapping);
    animateMic();
    FastLED.show();
  }

}
