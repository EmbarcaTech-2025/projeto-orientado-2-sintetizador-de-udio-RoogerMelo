#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

extern uint16_t  bufferAudio[NUM_AMOSTRAS];
extern volatile bool terminou_gravacao;

void  audio_init(void);
void  gravar_audio(void);
void  processar_deadzone_adaptativo(void);
void  reproduzir_audio(void);
