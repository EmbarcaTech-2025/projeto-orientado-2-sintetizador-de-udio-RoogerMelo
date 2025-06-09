#include "pico/stdlib.h"
#include "config.h"
#include "controls.h"

static uint64_t tick = 0;

void controls_init(void)
{
    gpio_init(BOTAO_A); gpio_set_dir(BOTAO_A, GPIO_IN); gpio_pull_up(BOTAO_A);
    gpio_init(BOTAO_B); gpio_set_dir(BOTAO_B, GPIO_IN); gpio_pull_up(BOTAO_B);

    gpio_init(LED_VERMELHO); gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_init(LED_VERDE);    gpio_set_dir(LED_VERDE,    GPIO_OUT);
}

static bool debounce(void)
{
    uint64_t now = time_us_64();
    if (now - tick < 200000) return false;
    tick = now;
    return true;
}

bool button_pressed_A(void) { return !gpio_get(BOTAO_A) && debounce(); }
bool button_pressed_B(void) { return !gpio_get(BOTAO_B) && debounce(); }

void led_rec (bool on){ gpio_put(LED_VERMELHO, on); }
void led_play(bool on){ gpio_put(LED_VERDE,   on); }
