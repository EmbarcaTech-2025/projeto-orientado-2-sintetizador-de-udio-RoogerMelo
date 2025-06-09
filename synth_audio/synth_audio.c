#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define SDA_PIN 14
#define SCL_PIN 15
#define LARGURA_TELA 128
#define ALTURA_TELA 64
#define PINO_MIC 28   
#define CANAL_ADC_MIC 2
#define PINO_PWM_BUZZER 21
#define BOTAO_A  5 
#define BOTAO_B  6
#define LED_VERMELHO 13
#define LED_AZUL 12
#define LED_VERDE 11
#define TAXA_AMOSTRAGEM_HZ 8000
#define DURACAO_SEGUNDOS 5
#define NUM_AMOSTRAS (TAXA_AMOSTRAGEM_HZ * DURACAO_SEGUNDOS)
#define MEIO 2048

static uint16_t bufferAudio[NUM_AMOSTRAS];
volatile bool terminou_gravacao = false;
static uint slice_pwm;
uint8_t buffer[ssd1306_buffer_length];

bool debouncing() {
    static uint64_t ultimo_tempo = 0;
    uint64_t agora = time_us_64();
    
    if (agora - ultimo_tempo < 200000)
        return false;
    
    ultimo_tempo = agora;
    return true;
}

struct render_area frame_area = {
    .start_column = 0,
    .end_column = ssd1306_width - 1,
    .start_page = 0,
    .end_page = ssd1306_n_pages - 1
};


void iniciar_display() {
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    ssd1306_init();
    calculate_render_area_buffer_length(&frame_area);
    render_on_display(buffer, &frame_area);
}

void modelo_histograma(uint16_t valor_adc) {
    const int LARG_COL = 30;
    const int X0       = (LARGURA_TELA - LARG_COL) / 2;
    const int X1       = X0 + LARG_COL - 1;
    const int Y_BASE   = ALTURA_TELA - 1;


    for (int x = X0; x <= X1; x++)
        for (int y = 0; y < ALTURA_TELA; y++)
            ssd1306_set_pixel(buffer, x, y, false);


    int32_t delta = abs((int32_t)valor_adc - MEIO);
    int alt = (delta * ALTURA_TELA) / (MEIO/2);
    if (alt > ALTURA_TELA) alt = ALTURA_TELA;
    int y_top = Y_BASE - alt + 1;


    for (int x = X0; x <= X1; x++)
        for (int y = y_top; y <= Y_BASE; y++)
            ssd1306_set_pixel(buffer, x, y, true);


    render_on_display(buffer, &frame_area);
}


void setup() {
    stdio_init_all();
    sleep_ms(500);
    iniciar_display();

    // ADC
    adc_init();
    adc_gpio_init(PINO_MIC);
    adc_select_input(CANAL_ADC_MIC);

    //CONF PWM
    gpio_set_function(PINO_PWM_BUZZER, GPIO_FUNC_PWM);
    slice_pwm = pwm_gpio_to_slice_num(PINO_PWM_BUZZER);
    pwm_set_wrap(slice_pwm, 4095);

    float div = clock_get_hz(clk_sys) / (20000.0f * 4096.0f);
    pwm_set_clkdiv(slice_pwm, div);
    pwm_set_enabled(slice_pwm, true);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_put(LED_VERMELHO, 0);

    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_put(LED_VERDE, 0);

    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);
    gpio_put(LED_AZUL, 0);
}


void gravar_audio() {
    terminou_gravacao = false;
    gpio_put(LED_VERMELHO, 1);
    uint64_t t0 = to_ms_since_boot(get_absolute_time());

    for (uint32_t i = 0; i < NUM_AMOSTRAS; i++) {
         
        bufferAudio[i] = adc_read();

        if (to_ms_since_boot(get_absolute_time()) - t0 > 100) {
            modelo_histograma(bufferAudio[i]);
            t0 = to_ms_since_boot(get_absolute_time());
        }
        sleep_us(1000000 / TAXA_AMOSTRAGEM_HZ);
    }

    gpio_put(LED_VERMELHO, 0);
    printf("Gravação concluída!\n");
    memset(buffer, 0, ssd1306_buffer_length);
    render_on_display(buffer, &frame_area);
    terminou_gravacao = true;
}
 


int zona_morta() {
    uint32_t soma = 0;
    for (uint32_t i = 0; i < NUM_AMOSTRAS; i++) {
        soma += abs((int32_t)bufferAudio[i] - MEIO);
    }
    int media = soma / NUM_AMOSTRAS;
    int deadLimit = media * 3;
    if (deadLimit <  50)  deadLimit =  50;
    if (deadLimit > 500)  deadLimit = 500;
    return deadLimit;
}

// Processa todo o buffer usando dead zone adaptativa + ganho
void processar_deadzone_adaptativo() {
    int dead = zona_morta();
    float ganho = 3;
    for (uint32_t i = 0; i < NUM_AMOSTRAS; i++) {
        int32_t x = (int32_t)bufferAudio[i] - MEIO;
        if (abs(x) < dead) {
            x = 0;
        } else {
            if (x > 0)      x = (x - dead) * ganho;
            else            x = (x + dead) * ganho;
        }
        
        if (x >  MEIO) x =  MEIO;
        if (x < -MEIO) x = -MEIO;
        
        bufferAudio[i] = (uint16_t)(x + MEIO);
    }
}

void reproduzir_audio() {
    if (!terminou_gravacao) return;

    int dead = zona_morta();
    printf("Iniciando reprodução...%d\n", dead);
    pwm_set_enabled(slice_pwm, true);  
    gpio_put(LED_VERDE, 255);

    for (uint32_t i = 0; i < NUM_AMOSTRAS; i++) {
        pwm_set_chan_level(slice_pwm, PWM_CHAN_B, bufferAudio[i]);
        sleep_us(1000000 / TAXA_AMOSTRAGEM_HZ);
    }

    pwm_set_chan_level(slice_pwm, PWM_CHAN_B, 0);
    pwm_set_enabled(slice_pwm, false);

    gpio_put(LED_VERDE, 0);
    printf("=== Reprodução concluída! ===\n");
}


int main() {
    setup();

    // lê estados iniciais (pull-up ativo → gpio_get = 1 quando solto)
    bool ultimoA = !gpio_get(BOTAO_A);
    bool ultimoB = !gpio_get(BOTAO_B);

    while (1) {
        // lê botão A (inverte porque pull-up → 0 quando pressionado)
        bool atualA = !gpio_get(BOTAO_A);
        // detecção de borda de descida: só dispara quando passa de “solto” para “pressionado”
        if (atualA && !ultimoA && !terminou_gravacao && debouncing()) {
            printf("Botão A: iniciando gravação\n");
            gravar_audio();
        
        }
        ultimoA = atualA;

        bool atualB = !gpio_get(BOTAO_B);
        // só dispara se a gravação já terminou
        if (atualB && !ultimoB && terminou_gravacao && debouncing()) {
            printf("Botão B: iniciando reprodução\n");
            reproduzir_audio();  // bloqueante
            // opcional: zera terminou_gravacao se quiser gravar de novo
            terminou_gravacao = false;
            ultimoA=!atualA;
        }

        tight_loop_contents();
    }
}