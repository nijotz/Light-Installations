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

void animateInnerTriangles(){
  /*  Set the LED values based on the left and right stacks
   *  This is a reverse loop because the left side LED's travel toward
   *  LED 0.
   */
  for (int i = 0; i < NUM_LEDS; i++) {
    // TODO: average colors properly
    //int lower_idx = floor(leds_inner_mapping[i]);
    //int upper_idx = ceil(leds_inner_mapping[i]);
    //float lower = sound_array[lower_idx];
    //float upper = sound_array[upper_idx];
    //float value = (lower + upper) / 2;
    int sound_idx = round(leds_inner_mapping[i]);
    set_LED_color(i, leds_inner_values, sound_wave[sound_idx]);
  }

  // Show the new LED values
  FastLED.show();
}

/* Sets led 'position' to 'value' and converts the value to an HSV value.
 * Compared to the A3 code, this code produces color values closer to white.
 * The A3 code always has only two colors lit at a time. For example red and 
 * green but not blue or blue and red but not green.
 * As a result, the colors are more like pastel hues than bold hues.
 * Another option would be to do something similar to the limited RGB values from A3.
 */
void set_LED_color(int position, CRGB leds[], int value) {
  // If lower than min amplitude, set to min amplitude
  if(value <= MIN_AMPLITUDE) {
    value = MIN_AMPLITUDE;
  }
  
  // Subtract min amplitude so lowest value is 0
  value -= MIN_AMPLITUDE;
  
  float max = (max_amplitude - MIN_AMPLITUDE);
  if(value > max) value = max;
  float ratio = ((float)(value / max));
  if(ratio == 0) {
    color.val = 0;
  } else {
    color.val = 255;
  }
  color.saturation = 255;
  color.hue = start_hue + ((ratio * 255) * 2);
  leds[position] = color;
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


/*  Push a new LED color value onto the beginning of the stack.
 *  The last LED color value is discarded. This is the primary 
 *  function relating to the propagation behavior.
 */
void push_stack(int stack[], int value) {
  int i;
  for(i = (LED_STACK_SIZE - 1); i >= 0; --i) {
    stack[i] = stack[i - 1];
  }
  stack[0] = value;
}
