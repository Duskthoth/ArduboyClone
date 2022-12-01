//================================================================================ //
// MUSICAS COM O ATMEGA //
// Leitura de arquivos RTTTL - Ringtones //
//================================================================================ //
 //frequ�ncia de opera��o de 16MHz 
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <menu.h>
#include <ssd1306.h>
//Defini��es de macros
#define set_bit(Y,bit_x) (Y|=(1<<bit_x)) //ativa bit 
#define clr_bit(Y,bit_x) (Y&=~(1<<bit_x)) //limpa bit 

uint8_t addr = SSD1306_ADDRESS;
//RTTTL
//------------------------------------------------------------------------------------
int main() {

//-----------------------------------------------------------------------------
	while(1) { 
		menu(addr);
		_delay_ms(1000);
    }
}
//==================================================================================

