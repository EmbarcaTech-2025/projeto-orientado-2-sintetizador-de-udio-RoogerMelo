#pragma once
#define SDA_PIN          14
#define SCL_PIN          15
#define LARGURA_TELA     128
#define ALTURA_TELA      64

#define PINO_MIC         28
#define CANAL_ADC_MIC    2
#define PINO_PWM_BUZZER  21

#define BOTAO_A          5
#define BOTAO_B          6

#define LED_VERMELHO     13
#define LED_AZUL         12
#define LED_VERDE        11

#define TAXA_AMOSTRAGEM_HZ 8000
#define DURACAO_SEGUNDOS   5
#define NUM_AMOSTRAS       (TAXA_AMOSTRAGEM_HZ * DURACAO_SEGUNDOS)
#define MEIO               2048
