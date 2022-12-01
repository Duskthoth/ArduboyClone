#include <avr/io.h> //definições do componente especificado
#include <avr/pgmspace.h>
#include <util/delay.h>
//#include <Arduino.h>
// include libraries
#include <stdio.h>
#include <ssd1306.h>
#include <stdlib.h>

//=====================Definições===================================
#define set_bit(Y, bit_x) (Y |= (1 << bit_x))  // ativa o bit x da variável Y (coloca em 1)
#define clr_bit(Y, bit_x) (Y &= ~(1 << bit_x)) // limpa o bit x da variável Y (coloca em 0)
#define tst_bit(Y, bit_x) (Y & (1 << bit_x))   // testa o bit x da variável Y (retorna 0 ou 1)
//#define cpl_bit(Y, bit_x) (Y ^= (1 << bit_x))  // troca o estado do bit x da variável Y (complementa)
#define BOTAO_UP PB4
#define BOTAO_DOWN PB3
#define SELECT PB2
// define configuracoes som
#define som PB1               // pino OC1A para sa�da do sinal
#define d_inic 4              // valor inicial de dura��o da nota musical
#define o_inic 5              // valor inicial da oitava
#define b 192                /*o nr de batidas indica a velocidade da musica (alterar para mudar a velocidade), maior = mais r�pido*/
#define t_min (7500 / b) * 10 // tempo m�nimo para formar o tempo base das notas musicais (1/32)
// define configuracoes jogo
#define true 1
#define false 0
#define nada -1
#define tam_plat 15

// variaveis para o pong
int player1_x = 9;
int player1_y = 26;
int player2_x = 118;
int player2_y = 26;
int ball_x = 62;
int ball_y = 30;
int score_pl_1 = 0;
int score_pl_2 = 0;
int invert_ball_y = true;
int invert_ball_x = true;
// int velocity_y = 1;
// int velocity_x = 1;
int start_game = false;

// functions for pong
void drawBox(void);
void draw_player_1(int);
void draw_player_2(int);
void drawBall(void);
void update_ball_x(void);
void update_ball_y(void);
void invert_ball_direction(void);
void draw_match_numbers(void);
void game_over_screen(uint8_t);
void init_screen(uint8_t);
void pong_init(uint8_t);
void test_colision(uint8_t, uint8_t,int8_t);
void toca_musica(const char *);
// musica
const char musica[] PROGMEM = {"8f,8f"};

void pong_init(uint8_t address)
{

  int addr = address;
  DDRB = 0b00000000;  // portas 2,1 e 0 como entrada;
  PORTB = 0b11111111; // habilita pullup para entradas
  // inicia pong
  SSD1306_ClearScreen();
  // INICIA SETUP DO JOGO
  while (1)
  {
    init_screen(addr);

    if (!tst_bit(PINB, BOTAO_UP)) // volta para menu principal
    {
      break;
    }
    if (!tst_bit(PINB, SELECT)) // inicia o jogo
    {
      start_game = true;
    }

    if (start_game) // o jogo
    {
      while (1)
      {
        SSD1306_ClearScreen();
        drawBox();
        draw_match_numbers();
        draw_player_1(addr);
        draw_player_2(addr);
        drawBall();

        if (!tst_bit(PINB, BOTAO_DOWN) && ((player1_y + tam_plat) < 62))
        {
          player1_y++;
        }
        if (!tst_bit(PINB, BOTAO_UP) && (player1_y > 1))
        {
          player1_y--;
        }
        if ((score_pl_1 == 3) || (score_pl_2 == 3))
        {
          start_game = 0;
          break;
        }
        SSD1306_UpdateScreen(addr);
      }
      if (start_game == 0)
      {
        game_over_screen(addr);
        break;
      }
    }
  }
}

// desenha arena
void drawBox()
{
  SSD1306_DrawLine(0, MAX_X, 0, 0);
  SSD1306_DrawLine(0, MAX_X, 63, 63);
  SSD1306_DrawLine(0, 0, 0, MAX_Y);
  SSD1306_DrawLine(127, 127, 0, MAX_Y);
}

// desenha o player 1
void draw_player_1(int addr)
{
  SSD1306_DrawLine(player1_x, player1_x, (player1_y), (player1_y + tam_plat));
  // SSD1306_UpdateScreen(addr);
}

// desenha a barra do jogador numero 2
void draw_player_2(int addr)
{
  // SSD1306_ClearScreen();

  // SSD1306_UpdateScreen(addr);
  if (ball_x > 74) // controla ia do pl2
  {
    if ((ball_y < player2_y) && (player2_y > 1))
    {
      player2_y--;
    }
    if ((ball_y > player2_y) && ((player2_y + tam_plat) < 62))
    {
      player2_y++;
    }
  }
  SSD1306_DrawLine(player2_x, player2_x, (player2_y), (player2_y + tam_plat));
}

// desenha a bola
void drawBall()
{
  invert_ball_direction();
  SSD1306_DrawPixel(ball_x, ball_y);
}

// atualiza posicao x da bola caso bata em algum dos players
void update_ball_x()
{
  if (!invert_ball_x)
  {
    ball_x--;
  }
  else if (invert_ball_x)
  {
    ball_x++;
  }
}

// atualiza a direçao da bola em y
void update_ball_y()
{
  if (!invert_ball_y)
  {
    ball_y--;
  }
  else if (invert_ball_y)
  {
    ball_y++;
  }
  else if (invert_ball_y == nada)
  {
    ball_y = ball_y;
  }
}

// inverte direcao da bola
void invert_ball_direction()
{
  // verifica se a bola bateu em alguma das paredes horizontais
  if ((ball_y == 1) || (ball_y == 62))
  {
    // se sim, inverte sua direcao
    invert_ball_y = !invert_ball_y;
  }
  // verifica se a bola chegou na coluna dos players
  test_colision(player1_x, player1_y,1);
  test_colision(player2_x, player2_y,-1);
  // testa se a bola ultrapassou os players, ou seja, nao houve colisao
  if ((ball_x == 1) || (ball_x == 126))
  {
    if (ball_x == 1)
    {
      score_pl_2++;
    }
    else if (ball_x == 126)
    {
      score_pl_1++;
    }
    ball_x = MAX_X / 2;
    ball_y = MAX_Y / 2;

    invert_ball_x = !invert_ball_x;
    invert_ball_y = !invert_ball_y;
  }
  update_ball_x();
  update_ball_y();
}

// desenha placar
void draw_match_numbers(void)
{
  SSD1306_DrawCharAt(42, 10, (char)score_pl_1);
  SSD1306_DrawCharAt(84, 10, (char)score_pl_2);
}

// desenha tela de game over
void game_over_screen(uint8_t address)
{
  int addr = address;
  SSD1306_ClearScreen();
  drawBox();
  if (score_pl_1 == 3)
  {
    SSD1306_SetPosition(40, 1);
    SSD1306_DrawString("GAME OVER");
    SSD1306_DrawLine(7, 120, 18, 18);
    SSD1306_SetPosition(6, 4);
    SSD1306_DrawString("PLAYER 1 WINS");
  }
  else if (score_pl_2 == 3)
  {
    SSD1306_SetPosition(40, 1);
    SSD1306_DrawString("GAME OVER");
    SSD1306_DrawLine(7, 120, 18, 18);
    SSD1306_SetPosition(6, 4);
    SSD1306_DrawString("PLAYER 2 WINS");
  }
  SSD1306_UpdateScreen(addr);
  _delay_ms(2000);
  SSD1306_ClearScreen();
  // redefine variaveis

  player1_x = 9;
  player1_y = 26;
  player2_x = 118;
  player2_y = 26;
  ball_x = 62;
  ball_y = 30;
  score_pl_1 = 0;
  score_pl_2 = 0;
  invert_ball_y = true;
  invert_ball_x = true;
  // velocity_y = 1;
  // velocity_x = 1;
  start_game = false;
}

// desenha tela inicial
void init_screen(uint8_t address)
{
  int addr = address;
  drawBox();
  SSD1306_SetPosition(40, 1);
  SSD1306_DrawString("PONG GAME");
  SSD1306_DrawLine(7, 120, 18, 18);
  SSD1306_SetPosition(4, 4);
  SSD1306_DrawString("PRESS 3 TO BEGIN");
  SSD1306_SetPosition(4, 6);
  SSD1306_DrawString("PRESS 1 TO EXIT");
  SSD1306_UpdateScreen(addr);
}

void test_colision(uint8_t player_x, uint8_t player_y,int8_t ajuste)
{
  int player_y_end = (tam_plat + player_y);
  if (player_x + ajuste == ball_x)
  {
    /* int fraction_size_paddle = tam_plat/3;
     if((player_y <= ball_y) && ( (player1_y+fraction_size_paddle)>= ball_y)){
       invert_ball_x = !invert_ball_x;
       invert_ball_y = false;
     }
     else if(((player_y+fraction_size_paddle) <= ball_y) && ( (player1_y+(2*fraction_size_paddle))>= ball_y)){
       invert_ball_x = !invert_ball_x;
       invert_ball_y = nada;
     }
     else if(((player_y+(2*fraction_size_paddle)) <= ball_y) && ( (player1_y+tam_plat)>= ball_y)){
       invert_ball_x = !invert_ball_x;
       invert_ball_y = true;
     }
   }*/
    if (ball_y >= player_y && ball_y <= player_y_end)
    {
      invert_ball_x = !invert_ball_x;
       toca_musica(musica);
    }

  }
}

void toca_musica(const char *musica)
{
  // configura e define a musica de inicio
  unsigned int k;
  unsigned char d, o, j, n, i = 0;
  DDRB |= (1 << som); // habilita a sa�da de som

  OCR1A = 18181;

  // TCCR1A = 1<<COM1A0;
  // TC1 modo CTC compara��o com OCR1A, prescaler=1
  TCCR1B = (1 << WGM12) | (1 << CS10);
  //------------------------------------------------------------------------------
  // LEITURA E EXECU��O DO ARQUIVO RTTTL
  //------------------------------------------------------------------------------
  do
  {
    o = o_inic; // carrega o valor default para a oitava
    if ((pgm_read_byte(&musica[i]) == '3') && (pgm_read_byte(&musica[i + 1]) == '2'))
    {
      d = 32;
      i += 2;
    }
    else if ((pgm_read_byte(&musica[i]) == '1') && (pgm_read_byte(&musica[i + 1]) == '6'))
    {
      d = 16;
      i += 2;
    }
    else if (pgm_read_byte(&musica[i]) == '8')
    {
      d = 8;
      i++;
    }
    else if (pgm_read_byte(&musica[i]) == '4')
    {
      d = 4;
      i++;
    }
    else if (pgm_read_byte(&musica[i]) == '2')
    {
      d = 2;
      i++;
    }
    else if (pgm_read_byte(&musica[i]) == '1')
    {
      d = 1;
      i++;
    }
    else
      d = d_inic; // carrega o valor dafault para a dura��o
    if (pgm_read_byte(&musica[i + 1]) == '#')
    {
      switch (pgm_read_byte(&musica[i]))
      { // carrega a oitava # default (4a)
      case 'a':
        OCR1A = 17159;
        break; // A# - L�#
      case 'c':
        OCR1A = 14429;
        break; // C# - D�#
      case 'd':
        OCR1A = 12852;
        break; // D# - R�#
      case 'f':
        OCR1A = 10807;
        break; // F# - F�#
      case 'g':
        OCR1A = 9627;
        break; // G# - S�l#
      }
      i += 2;
    }
    else
    {
      switch (pgm_read_byte(&musica[i]))
      { // carrega a oitava default (4a)
      case 'a':
        OCR1A = 18181;
        break; // A - L�
      case 'b':
        OCR1A = 16197;
        break; // B - Si
      case 'c':
        OCR1A = 15287;
        break; // C - D�
      case 'd':
        OCR1A = 13618;
        break; // D - R�
      case 'e':
        OCR1A = 12131;
        break; // E - Mi
      case 'f':
        OCR1A = 11449;
        break; // F - F�
      case 'g':
        OCR1A = 10199;
        break; // G - Sol
      case 'p':
        clr_bit(TCCR1A, COM1A0);
        break; // p = pausa
      }
      i++;
    }
    n = 32 / d; // tempo de dura��o de cada nota musical

    if (pgm_read_byte(&musica[i]) == '.')
    {
      n = n + (n / 2); // dura��o 50% >
      i++;
    }
    if (pgm_read_byte(&musica[i]) == '4')
    {
      o = 4;
      i++;
    }
    else if (pgm_read_byte(&musica[i]) == '5')
    {
      o = 5;
      i++;
    }
    else if (pgm_read_byte(&musica[i]) == '6')
    {
      o = 6;
      i++;
    }
    else if (pgm_read_byte(&musica[i]) == '7')
    {
      o = 7;
      i++;
    }
    if (pgm_read_byte(&musica[i]) == '.')
    {
      n = n + (n / 2); // dura��o 50% >
      i++;
    }
    switch (o)
    { // troca a oitava se n�o for a default (o = 4)
    case 5:
      OCR1A = OCR1A >> 1;
      break; // divide por 2
    case 6:
      OCR1A = OCR1A >> 2;
      break; // divide por 4
    case 7:
      OCR1A = OCR1A >> 4;
      break; // divide por 8
    }
    //------------------------------------------------------------------------
    for (j = 0; j < n; j++)
    { // nr de repeti��es para a nota 1/32
      for (k = t_min; k != 0; k--)
        _delay_us(100);
    }
    //------------------------------------------------------------------------
    set_bit(TCCR1A, COM1A0);                    // habilita o som
  } while (pgm_read_byte(&musica[i++]) == ','); // leitura at� o final da m�sica
  TCCR1A = 0;                                   // desabilita o som e o TC1
  TCCR1B = 0;
}