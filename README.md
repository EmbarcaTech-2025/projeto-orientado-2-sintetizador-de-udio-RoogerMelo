# Projetos de Sistemas Embarcados – EmbarcaTech 2025

**Autor:** Roger Melo  
**Curso:** Residência Tecnológica em Sistemas Embarcados  
**Instituição:** EmbarcaTech - HBr  
Campinas, ___ de 2025  

---

## 📼 Demonstração em vídeo
[![Assista no YouTube](https://img.shields.io/badge/YouTube-Ver%20demo-red?logo=youtube)](https://youtube.com/shorts/z0TksL6Grco?feature=share)

> *Clique no badge acima para ver o projeto em ação.*

---

## 🗂️ Sumário
1. [Descrição do projeto](#descrição-do-projeto)  
2. [Arquitetura de hardware](#arquitetura-de-hardware)  
3. [Estrutura de código](#estrutura-de-código)  
4. [Licença](#licença)

---

## Descrição do projeto
Breve resumo: qual problema o sistema resolve, principais recursos (ex.: captura de áudio, exibição em OLED, reprodução via PWM) e metas de desempenho.

## Arquitetura de hardware
- **Microcontrolador:** Raspberry Pi Pico W (RP2040)  
- **Periféricos:** MAX4466 (microfone), SSD1306 (OLED I²C), buzzer passivo, botões (GPIO 5/6), LEDs (GPIO 11–13)

## Estrutura de código
```text
.
├── app/
│   └── main.c
├── hal/
│   ├── audio.c
│   ├── display.c
│   └── controls.c
├── drivers/
│   └── ssd1306_i2c.c
├── include/
│   ├── audio.h
│   ├── config.h
│   ├── controls.h
│   └── display.h
└── CMakeLists.txt
