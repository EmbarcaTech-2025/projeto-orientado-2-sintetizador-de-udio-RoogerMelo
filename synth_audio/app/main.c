#include "pico/stdlib.h"
#include "config.h"
#include "display.h"
#include "controls.h"
#include "audio.h"

int main()
{
    stdio_init_all();
    display_init();
    controls_init();
    audio_init();

    while (true)
    {
        if (button_pressed_A() && !terminou_gravacao) {
            led_rec(true);
            gravar_audio();
            processar_deadzone_adaptativo();
            led_rec(false);
        }

        if (button_pressed_B() && terminou_gravacao) {
            led_play(true);
            reproduzir_audio();
            led_play(false);
            terminou_gravacao = false;
        }
        tight_loop_contents();
    }
}
