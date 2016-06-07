#include "Arduino.h"

void getAudiomsg(){

  // Reset EQ7 chip
  digitalWrite(RESET_PIN, HIGH);
  digitalWrite(RESET_PIN, LOW);

  // Change color mode if stomp button is pressed
  if(stomp_pressed()) {
    change_color_mode();
    delay(100); // Keeps the pedal from switching colors too quickly
  }

  // Get the sum of the amplitudes of all 7 frequency bands
  amp_sum_L = get_freq_sum(ANALOG_PIN_L);
  amp_sum_R = get_freq_sum(ANALOG_PIN_R);

  /* If monomode is active, make both L and R equal to the
     value of L */
  if(monomode) amp_sum_L = amp_sum_R;

  setsensitivity();


}

// Read in and sum amplitudes for the 7 frequency bands
int get_freq_sum(int pin) {

  int i;
  int spectrum_values[7];
  int spectrum_total = 0;

  //get readings from chip, sum freq values
  for (i = 0; i < 7; i++) {
    digitalWrite(STROBE_PIN, LOW);
    delayMicroseconds(30); // to allow the output to settle

    spectrum_values[i] = analogRead(pin);
    spectrum_total += spectrum_values[i];

    // strobe to the next frequency
    digitalWrite(STROBE_PIN, HIGH);

  }//for i
  return spectrum_total;
}

void setsensitivity(){
  /*  SENSITIVITY_DIVISOR lowers the range of the possible pot values
   *  If the value is higher than the max or lower than the min,
   *  set it to the max or min respectively.
   */
  sensitivity = (analogRead(SENSITIVITY_POT_PIN) / SENSITIVITY_DIVISOR) * SENSITIVITY_MULTIPLIER;
  max_amplitude = MAX_AMPLITUDE - (((1023 - analogRead(SENSITIVITY_POT_PIN)) / SENSITIVITY_DIVISOR) * MAX_AMPLITUDE_MULTIPLIER);

}

void updateSoundWave() {
  getAudiomsg();  // sets ampsum left and right value
  push_audio_stack(sound_buffer, amp_sum_L);
  push_color_stack(sound_wave, sound_buffer[0]);
}

/* Sets led 'position' to 'value' and converts the value to an HSV value.
 * Compared to the A3 code, this code produces color values closer to white.
 * The A3 code always has only two colors lit at a time. For example red and
 * green but not blue or blue and red but not green.
 * As a result, the colors are more like pastel hues than bold hues.
 * Another option would be to do something similar to the limited RGB values from A3.
 */
CRGB get_LED_color(int value) {
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
  return color;
}

void push_audio_stack(int stack[], int value) {
  int sum = 0;
  for (int i = (SOUND_BUFFER_LENGTH - 1); i >= 0; --i) {
    sum += stack[i];
    stack[i] = stack[i - 1];
  }
  sum += value;
  stack[0] = sum / (SOUND_BUFFER_LENGTH + 1);
}

void push_color_stack(CRGB stack[], int value) {
  for (int i = (SOUND_WAVE_LENGTH - 1); i >= 0; --i) {
    stack[i] = stack[i - 1];
  }
  CRGB color = get_LED_color(value);
  CRGB avg_color;
  avg_color.r = (color.r + stack[1].r) / 2;
  avg_color.g = (color.g + stack[1].g) / 2;
  avg_color.b = (color.b + stack[1].b) / 2;
  stack[0] = avg_color;
}
