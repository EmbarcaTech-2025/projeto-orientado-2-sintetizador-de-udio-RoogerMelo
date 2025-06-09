#include <stdlib.h>   /* abs */
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "config.h"
#include "audio.h"
#include "display.h"

uint16_t  bufferAudio[NUM_AMOSTRAS];
volatile bool terminou_gravacao = false;
static uint slice;

void audio_init(void)
{
    
    adc_init();
    adc_gpio_init(PINO_MIC);
    adc_select_input(CANAL_ADC_MIC);

   
    gpio_set_function(PINO_PWM_BUZZER, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(PINO_PWM_BUZZER);
    pwm_set_wrap(slice, 4095);
    float div = (float)clock_get_hz(clk_sys) / (20000.0f * 4096.0f);
    pwm_set_clkdiv(slice, div);
    pwm_set_enabled(slice, false);
}


void gravar_audio(void)
{
    terminou_gravacao = false;
    uint64_t t0 = to_ms_since_boot(get_absolute_time());

    for (uint32_t i = 0; i < NUM_AMOSTRAS; ++i)
    {
        bufferAudio[i] = adc_read();
        if (to_ms_since_boot(get_absolute_time()) - t0 > 100) {
            modelo_histograma(bufferAudio[i]);
            t0 = to_ms_since_boot(get_absolute_time());
        }
        sleep_us(1000000 / TAXA_AMOSTRAGEM_HZ);
    }
    display_clear();
    terminou_gravacao = true;
}


static int32_t dead_limit(void)
{
    uint32_t soma = 0;
    for (uint32_t i = 0; i < NUM_AMOSTRAS; ++i)
        soma += abs((int32_t)bufferAudio[i] - MEIO);

    int d = (soma / NUM_AMOSTRAS) * 3;
    if (d < 50) d = 50; if (d > 500) d = 500;
    return d;
}

void processar_deadzone_adaptativo(void)
{
    const float GAIN = 3.0f;
    int dead = dead_limit();

    for (uint32_t i = 0; i < NUM_AMOSTRAS; ++i)
    {
        int32_t x = (int32_t)bufferAudio[i] - MEIO;
        if (abs(x) < dead) x = 0;
        else               x = (x > 0) ? (x - dead) * GAIN
                                       : (x + dead) * GAIN;

        if (x >  MEIO) x =  MEIO;
        if (x < -MEIO) x = -MEIO;
        bufferAudio[i] = (uint16_t)(x + MEIO);
    }
}


void reproduzir_audio(void)
{
    if (!terminou_gravacao) return;

    pwm_set_enabled(slice, true);
    for (uint32_t i = 0; i < NUM_AMOSTRAS; ++i)
    {
        pwm_set_chan_level(slice, PWM_CHAN_B, bufferAudio[i]);
        sleep_us(1000000 / TAXA_AMOSTRAGEM_HZ);
    }
    pwm_set_chan_level(slice, PWM_CHAN_B, 0);
    pwm_set_enabled(slice, false);
}
