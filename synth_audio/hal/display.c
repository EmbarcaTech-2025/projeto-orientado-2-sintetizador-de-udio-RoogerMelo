#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "config.h"
#include "display.h"

uint8_t buffer[ssd1306_buffer_length];

static struct render_area full = {
    .start_column = 0,
    .end_column   = ssd1306_width  - 1,
    .start_page   = 0,
    .end_page     = ssd1306_n_pages - 1
};

void display_init(void)
{
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    ssd1306_init();
    display_clear();
}

void display_clear(void)
{
    memset(buffer, 0, ssd1306_buffer_length);
    render_on_display(buffer, &full);
}


void modelo_histograma(uint16_t adc_val)
{
    const int L = 30;
    const int X0 = (LARGURA_TELA - L) / 2;
    const int X1 = X0 + L - 1;
    const int YB = ALTURA_TELA - 1;

    for (int x = X0; x <= X1; ++x)
        for (int y = 0; y < ALTURA_TELA; ++y)
            ssd1306_set_pixel(buffer, x, y, false);

    int32_t delta = abs((int32_t)adc_val - MEIO);
    int alt = (delta * ALTURA_TELA) / (MEIO / 2);
    if (alt > ALTURA_TELA) alt = ALTURA_TELA;

    int y_top = YB - alt + 1;
    for (int x = X0; x <= X1; ++x)
        for (int y = y_top; y <= YB; ++y)
            ssd1306_set_pixel(buffer, x, y, true);

    render_on_display(buffer, &full);
}
