#include <avr/io.h> //definições do componente especificado
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>
//#include <Arduino.h>
// include libraries
#include <stdio.h>
#include <ssd1306.h>
#include <stdlib.h>
#include <pong.h>
#include <space.h>

#define tst_bit(Y, bit_x) (Y & (1 << bit_x)) // testa o bit x da variável Y (retorna 0 ou 1)

#define BOTAO_UP PB4
#define BOTAO_DOWN PB3
#define SELECT PB2


void draw_Screen(uint8_t);

void menu(int address)
{
    int addr = address;
    DDRB = 0b00000000;  // portas 2,1 e 0 como entrada;
    PORTB = 0b11111111; // habilita pullup para entradas

    SSD1306_Init(addr);
    SSD1306_NormalScreen(addr);
    draw_Screen(addr);
    while (1)
    {

        if (!tst_bit(PINB, BOTAO_UP))
        {
            _delay_ms(1000);
            pong_init(addr);
            SSD1306_ClearScreen();
            break;
        }
        else if (!tst_bit(PINB, BOTAO_DOWN))
        {
           
            space_init(addr);
            SSD1306_ClearScreen();
             _delay_ms(1000);
            break;
        }
        else if (!tst_bit(PINB, SELECT))
        {
        }
    }
}

void draw_Screen(uint8_t addr)
{
    SSD1306_DrawLine(0, MAX_X, 0, 0);
    SSD1306_DrawLine(0, MAX_X, 63, 63);
    SSD1306_DrawLine(0, 0, 0, MAX_Y);
    SSD1306_DrawLine(127, 127, 0, MAX_Y);
    SSD1306_SetPosition(10, 1);
    SSD1306_DrawString("BOTAO_1: PONG GAME");
    SSD1306_DrawLine(7, 120, 18, 18);
    SSD1306_SetPosition(10, 3);
    SSD1306_DrawString("BOTAO_2: SPACE GAME");
    SSD1306_DrawLine(7, 120, 36, 36);
    SSD1306_SetPosition(10, 6);
    SSD1306_DrawString("By. Andrew e Felipe");
    SSD1306_UpdateScreen(addr);
}