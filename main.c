#include <stdio.h> // Biblioteca padrão
#include <math.h> // Biblioteca de matemática (função "round" foi utilizada)
#include "pico/stdlib.h" // Biblioteca padrão pico
#include "hardware/gpio.h" // Biblioteca de GPIOs
#include "hardware/adc.h" // Biblioteca do ADC
#include "hardware/pwm.h" // Biblioteca do PWM

#define MIC_PIN 28

int main() {
  adc.init(MIC_PIN);
  return 0;
}