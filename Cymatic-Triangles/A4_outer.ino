#include "Arduino.h"
#include <math.h>

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

void animateOuterTriangles(){
  /*  Set the LED values based on the left and right stacks
   *  This is a reverse loop because the left side LED's travel toward
   *  LED 0.
   */
  for (int i = 0; i < NUM_LEDS; i++) {
    // TODO: average colors properly
    //int lower_idx = floor(leds_outer_mapping[i]);
    //int upper_idx = ceil(leds_outer_mapping[i]);
    //float lower = sound_array[lower_idx];
    //float upper = sound_array[upper_idx];
    //float value = (lower + upper) / 2;
    int sound_idx = round(leds_outer_mapping[i]);
    set_LED_color(i, leds_outer_values, sound_wave[sound_idx]);
  }

  // Show the new LED values
  FastLED.show();
}
