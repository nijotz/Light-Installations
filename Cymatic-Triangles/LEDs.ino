#include "Arduino.h"
#include <math.h>

void setupInnerTriangleMapping() {
  float SIDE = NUM_LEDS / 3;
  float HALF_SIDE = SIDE / 2;
  float SIN_60 = 0.866;
  float HEIGHT = SIN_60 * SIDE;
  float SQUARE_HEIGHT = pow(HEIGHT, 2);

  // For left(?) side coming from the center
  for (int i = 0; i < SIDE; i++) {
    leds_inner_mapping[i] = i;
  }

  // For the other side coming from the center
  for (int i = 0; i < SIDE; i++) {
    leds_inner_mapping[NUM_LEDS - i] = i;
  }

  // For the bottom side of the inner triangle
  for (int i = 0; i < HALF_SIDE; i++) {
    // A^2 + B^2 = C^2
    // A = HEIGHT
    // B = HALF_SIDE
    // C = HYPOTONUSE -- where on the sound wave array to pull values from

    // Doesn't matter left vs right, they get the same values
    int left_side = SIDE + i;
    int right_side = 2 * SIDE - i;
    float bottom_mapping =  sqrt(SQUARE_HEIGHT + pow(HALF_SIDE - i, 2));

    leds_inner_mapping[right_side] = bottom_mapping;
    leds_inner_mapping[left_side] = bottom_mapping;
  }
}

void setupOuterTriangleMapping() {
  float SIDE = NUM_LEDS / 3;
  float HALF_SIDE = SIDE / 2;
  float SIN_60 = 0.866;
  float SIN_30 = 0.5;
  float COS_30 = 0.866;
  float HEIGHT = SIN_60 * SIDE;

  // For the top side of the outer triangle
  for (int i = 0; i < HALF_SIDE; i++) {
    // A^2 + B^2 = C^2
    // A = HEIGHT
    // B = HALF_SIDE
    // C = HYPOTONUSE -- where on the sound wave array to pull values from

    // Doesn't matter left vs right, they get the same values
    int left_side = SIDE + i;
    int right_side = 2 * SIDE - i;
    float bottom_mapping =  sqrt(pow(HEIGHT + 5, 2) + pow(HALF_SIDE - i, 2));

    leds_outer_mapping[right_side] = bottom_mapping;
    leds_outer_mapping[left_side] = bottom_mapping;
  }

  // For the bottom sides of the outer triangle
  for (int i = 0; i < SIDE; i++) {
    int left_side = i;
    int right_side = NUM_LEDS - i;

    float from_bottom = float(i) * COS_30;
    float mapping = sqrt( pow(HEIGHT + 5 + HEIGHT - from_bottom, 2) + pow(i * SIN_30, 2) );

    leds_outer_mapping[right_side] = mapping;
    leds_outer_mapping[left_side] = mapping;
  }
}

void animateTriangles(CRGB values[], float mappings[]) {
  for (int i = 0; i < NUM_LEDS; i++) {
    int mapping = mappings[i] * SOUND_LED_SCALE;
    values[i] = sound_wave[mapping];
  }
}

// Check if stomp button is being pressed
int stomp_pressed() {
  if (digitalRead(STOMP_PIN) == HIGH){
    return 1;
  } else {
    return 0;
  }
}

void change_color_mode() {
  start_hue += 85;
  if(start_hue >= 255) {
    start_hue = start_hue - 255;
  }
}
