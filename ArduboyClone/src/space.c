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
//#define cpl_bit(Y, bit_x) (Y ^= (1 << bit_x)) // troca o estado do bit x da variável Y (complementa)
#define BOTAO_UP PB4
#define BOTAO_DOWN PB3
#define SELECT PB2
// define configuracoes jogo
#define true 1
#define false 0
#define nada -1
#define TAM_TIRO 5  // Tamanho do tiro da nave 1
#define TAM_TIRO2 7 // Tamanho do tiro da nave 2

// define configuracoes som
#define som PB1               // pino OC1A para sa�da do sinal
#define d_inic 4              // valor inicial de dura��o da nota musical
#define o_inic 5              // valor inicial da oitava
#define b 192                 /*o nr de batidas indica a velocidade da musica (alterar para mudar a velocidade), maior = mais r�pido*/
#define t_min (7500 / b) * 10 // tempo m�nimo para formar o tempo base das notas musicais (1/32)

// variaveis para o space
int start_game2 = false;

int nave_x = 9;
int nave_y = 26;
int tam_nave = 11;

int nave_x2 = 95;
int nave_y2 = 16;
int tam_nave2 = 32;

int tiro_y = 0;
int tiro_x = 0;

int tiro2_y = 0;
int tiro2_x = 0;

uint8_t tiro_control = false;
uint8_t tiro2_control = false;

uint8_t nave2_control = false;

int score_nave1 = 3;
int score_nave2 = 5;

const char musica2[] PROGMEM = {"8f,8f"}; // musica

// Bitmaps
const uint8_t st_bitmap_player1[] =
    {
        0x06, 0x1e, 0x06, 0x07, 0x1f, 0x7e, 0x1f, 0x07, 0x06, 0x1e, 0x06};

const uint8_t st_bitmap_player2[] =
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80,
        0x80, 0xc0, 0xc0, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf8, 0xfc, 0xfe, 0xff, 0xff, 0xe3,
        0xc9, 0x1c, 0x9d, 0xc9, 0xe3, 0xff, 0xff, 0xfe, 0xfc, 0xf8, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x01, 0x01, 0x7f, 0x07, 0xff, 0x01,
        0x01, 0xfe, 0x07, 0xff, 0x01, 0xff, 0xff, 0x07, 0x3f, 0x1f, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
        0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const uint8_t st_bitmap_tiro_player2[] =
    {
        /* 00111000 */ 0x038,
        /* 01111100 */ 0x07c,
        /* 11111100 */ 0x0fc,
        /* 11111110 */ 0x0fe,
        /* 11111110 */ 0x0fe,
        /* 01111110 */ 0x07e,
        /* 01111000 */ 0x078};

// functions for space
void drawBox2(void);                                 // desenha bordas
void draw_nave(int);                                 // desenha nave 1
void draw_nave2(int);                                // desenha nave 2 e controla a I.A
void init_screen2(uint8_t);                          // apresenta a tela inicial do jogo
void space_init(uint8_t);                            // função principal
void draw_tiro(uint8_t, uint8_t);                    // desenha o tiro da nave 1 e atualiza a posicao e condição para atirar
void draw_tiro2(uint8_t, uint8_t);                   // desenha o tiro da nave 2 e atualiza a posicao e condição para atirar
void test_colision_nave1(uint8_t, uint8_t, uint8_t); // testa se o tiro da nave 1 colidiu com a nave 2
void test_colision_nave2(uint8_t, uint8_t, uint8_t); // testa se o tiro da nave 2 colidiu com a nave 1
void toca_musica2(const char *);                     // toca o som da colisão
void draw_match_numbers2(void);                      // desenha na tela a vida das naves
void game_over_screen2(uint8_t);                     // desenha a tela de fim de jogo e redefine as variaveis do jogo

void space_init(uint8_t address)
{

    int addr = address; // armazena o endereco de comandos da tela
    DDRB = 0b00000000;  // portas 2,1 e 0 como entrada;
    PORTB = 0b11111111; // habilita pullup para entradas
    // inicia space
    SSD1306_ClearScreen();
    // INICIA SETUP DO JOGO
    while (1)
    {
        init_screen2(addr);

        if (!tst_bit(PINB, BOTAO_UP)) // volta para menu principal
        {
            break;
        }
        if (!tst_bit(PINB, SELECT)) // inicia o jogo
        {
            start_game2 = true;
        }

        if (start_game2) // o jogo
        {
            while (1)
            {
                SSD1306_ClearScreen(); //limpa a tela
                drawBox2(); //desenha as bordas
                draw_nave(addr); //desenha a nave 1
                draw_nave2(addr); //desenha a nave 2
                draw_match_numbers2(); //desenha a vida das naves
                draw_tiro(addr, tiro_control); //desenha o tiro da nave 1
                draw_tiro2(addr, tiro2_control); //desenha o tiro da nave2
                test_colision_nave1(nave_x, nave_y, 1); //testa se o tiro da nave 2 colidiu com a nave 1
                test_colision_nave2(nave_x2, nave_y2, 2); //testa se o tiro da nave 1 colidiu com a nave 2

                if (!tst_bit(PINB, BOTAO_DOWN) && ((nave_y + tam_nave) < 62))
                { //testa para ver se o botao 2 mudou de estado
                    nave_y++; //se sim incrementa o valor de y da nave 1
                }
                if (!tst_bit(PINB, BOTAO_UP) && (nave_y > 2))
                { //testa para ver se o botão 1 mudou de estado
                    nave_y--; //se sim decrementa o valor do y da nave 1
                }
                if (!tst_bit(PINB, SELECT))
                { //testa para ver se o botão 3 mudou de estado
                    if (!tiro_control) //verifica se pode atirar
                    {
                        tiro_control = true; //se sim, altera o controle para true
                    }
                }
                if ((score_nave1 == 0) || (score_nave2 == 0))
                { //se a vida das naves chegar a 0, inicia o processo de fim de jogo
                    start_game2 = 0; //anula o estado do jogo
                    break; //sai do loop
                }

                SSD1306_UpdateScreen(addr); //atualiza a tela com os objetos desenhados
            }
            if (start_game2 == 0) //testa se deve ativar a tela de fim de jogo
            {
                game_over_screen2(addr); //caso a nave 1 tenha ganho, apresenta na tela, se nao, apresenta que foi a nave 2
                break; // finaliza o jogo e volta ao menu principal de jogos
            }
        }
    }
}

// desenha arena
void drawBox2()
{
    SSD1306_DrawLine(0, MAX_X, 0, 0);
    SSD1306_DrawLine(0, MAX_X, 63, 63);
    // SSD1306_DrawLine(0, 0, 0, MAX_Y);
    // SSD1306_DrawLine(127, 127, 0, MAX_Y);
}

// desenha a nave 1
void draw_nave(int addr)
{
    drawBitMap(nave_x, nave_y, st_bitmap_player1, 11, 1);
}

// desenha a nave 2
void draw_nave2(int addr)
{

    if (nave_y2 + 5 > 0 && nave2_control == false) // Se a posição y da nave for superior  0, eleva a nave
    {                                              //É adicionado +5 para a nave parar 5 pixels antes da barra superior
        nave_y2--;                                 // nave2_control para controle de prioridade da nave seguir pra uma direção
    }                                              // false: para nave subir e true: para nave descer.
    if (nave_y2 + 5 == 1)
    {
        nave2_control = true; // a nave chegou ao limite superior, então alterou o controle para poder descer
    }
    if ((nave_y2 + tam_nave2 - 3) < 63 && nave2_control == true) // Se a posição y da nave for inferior a 63, rebaixa a nave
    {                                                            //É adicionado -3 para a nave parar 3 pixels antes da barra inferior
        nave_y2++;                                               //+ o tam_nave2 para o desenho não ultrapassar o limite
    }                                                            // nave2_control para controle de prioridade da nave seguir pra uma direção
    if ((nave_y2 + tam_nave2 - 3) == 62)
    {
        nave2_control = false; // a nave chegou ao limite inferior, então alterou o controle para poder subir
    }

    drawBitMap(nave_x2, nave_y2, st_bitmap_player2, tam_nave2, 4); // chama o metódo para desenhar o bitmap (classe: SSD1306)
}

// desenha o tiro 1
void draw_tiro(uint8_t addr, uint8_t controle)
{
    // controle é iniciado como false
    if (!controle)
    {
        tiro_y = (nave_y + (tam_nave / 2)); // calcula para que o tiro seja disparado do meio da nave
        tiro_x = (nave_x + 8);              // calcula para que o tiro seja disparado da frente da nave
    }
    // Quando o botão é precinado o controle é alterado para true
    if (controle)
    {
        if (tiro_x + TAM_TIRO == 110) // Verifica se o tiro chegou no limite da tela
        {
            tiro_control = false; // Caso verdadeiro, apaga o desenho.
            _delay_ms(250);
        }
        SSD1306_DrawLine(tiro_x, (tiro_x + TAM_TIRO), tiro_y, tiro_y); // chama o metódo para desenhar a linha do tiro (classe: SSD1306)
        tiro_x++;                                                      // incrementa em x para movimentar o desenho
    }
}

// desenha o tiro 2
void draw_tiro2(uint8_t addr, uint8_t controle2)
{
    // controle é iniciado como false
    if (!controle2)
    {
        tiro2_y = (nave_y2 + (tam_nave2 / 2)); // calcula para que o tiro seja disparado do meio da nave
        tiro2_x = (nave_x2 - 2);               // calcula para que o tiro seja disparado da frente da nave
    }
    // Nave 2 atira no centro da nave 1
    if ((nave_y - 14) == nave_y2)
    {
        tiro2_control = true; // controle é alterado para true para o mesmo disparar
    }
    // controle é alterado para true quando a nave 1 estiver no centro da nave 2
    if (controle2)
    {
        if (tiro2_x == 7) // Verifica se o tiro chegou no limite da tela
        {
            tiro2_control = false; // Caso verdadeiro, apaga o desenho.
        }

        drawBitMap(tiro2_x, tiro2_y, st_bitmap_tiro_player2, TAM_TIRO2, 1); // chama o metódo para desenhar o bitmap (classe: SSD1306)
        tiro2_x--;                                                          // decrementa em x para movimentar o desenho da direita para esquerda
    }
}

// desenha na tela o placar
void draw_match_numbers2(void)
{
    SSD1306_DrawCharAt(42, 10, (char)score_nave1);
    SSD1306_DrawCharAt(84, 10, (char)score_nave2);
}

// desenha a tela de fim de jogo e redefine as variaveis do jogo
void game_over_screen2(uint8_t address)
{
    int addr = address;
    SSD1306_ClearScreen();
    drawBox2();
    if (score_nave1 == 0)
    {
        SSD1306_SetPosition(40, 1);
        SSD1306_DrawString("GAME OVER");
        SSD1306_DrawLine(7, 120, 18, 18);
        SSD1306_SetPosition(6, 4);
        SSD1306_DrawString("DEATH STAR WINS");
    }
    else if (score_nave2 == 0)
    {
        SSD1306_SetPosition(40, 1);
        SSD1306_DrawString("GAME OVER");
        SSD1306_DrawLine(7, 120, 18, 18);
        SSD1306_SetPosition(6, 4);
        SSD1306_DrawString("PLAYER WINS");
    }
    SSD1306_UpdateScreen(addr);
    _delay_ms(2000);
    SSD1306_ClearScreen();
    // redefine variaveis

    nave_x = 9;
    nave_y = 26;
    tam_nave = 11;

    tiro_y = 0;
    tiro_x = 0;

    tiro2_y = 0;
    tiro2_x = 0;

    tiro_control = false;
    tiro2_control = false;

    nave2_control = false;

    nave_x2 = 95;
    nave_y2 = 16;
    tam_nave2 = 32;

    score_nave1 = 3;
    score_nave2 = 5;

    start_game2 = false;
}

// apresenta a tela inicial do jogo
void init_screen2(uint8_t address)
{
    int addr = address;
    drawBox2();
    SSD1306_SetPosition(40, 1);
    SSD1306_DrawString("SPACE GAME");
    SSD1306_DrawLine(7, 120, 18, 18);
    SSD1306_SetPosition(4, 4);
    SSD1306_DrawString("PRESS 3 TO BEGIN");
    SSD1306_SetPosition(4, 6);
    SSD1306_DrawString("PRESS 1 TO EXIT");
    SSD1306_UpdateScreen(addr);
}

// testa se o tiro da nave 2 colidiu com a nave 1
void test_colision_nave1(uint8_t player_x, uint8_t player_y, uint8_t ajuste)
{
    int player_y_end = (tam_nave + player_y); // calcula o valor de y final para a nave
    if (player_x + ajuste == tiro2_x)         // define o pixel o qual o teste começa
    {
        if (tiro2_y >= player_y && tiro2_y <= player_y_end) // testa para ver se o tiro colidiu na nave 1
        {
            toca_musica2(musica2); // reproduz som de colisao
            score_nave1--;         // se, sim decrementa a vida da nave 1
        }
    }
}

// testa se o tiro da nave 1 colidiu com a nave 2
void test_colision_nave2(uint8_t player_x, uint8_t player_y, uint8_t ajuste)
{
    int player_y_end = (tam_nave2 + player_y - 2); // calcula o valor de y final para a nave
    if (player_x + ajuste == tiro_x)               // define o pixel o qual o teste começa
    {
        if (tiro_y >= player_y && tiro_y <= player_y_end) // testa para ver se o tiro colidiu na nave 2
        {
            toca_musica2(musica2); // reproduz som de colisao
            score_nave2--;         // se, sim decrementa a vida ds nave 2
        }
    }
}

// toca o som da colisão
void toca_musica2(const char *musica2)
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
        if ((pgm_read_byte(&musica2[i]) == '3') && (pgm_read_byte(&musica2[i + 1]) == '2'))
        {
            d = 32;
            i += 2;
        }
        else if ((pgm_read_byte(&musica2[i]) == '1') && (pgm_read_byte(&musica2[i + 1]) == '6'))
        {
            d = 16;
            i += 2;
        }
        else if (pgm_read_byte(&musica2[i]) == '8')
        {
            d = 8;
            i++;
        }
        else if (pgm_read_byte(&musica2[i]) == '4')
        {
            d = 4;
            i++;
        }
        else if (pgm_read_byte(&musica2[i]) == '2')
        {
            d = 2;
            i++;
        }
        else if (pgm_read_byte(&musica2[i]) == '1')
        {
            d = 1;
            i++;
        }
        else
            d = d_inic; // carrega o valor dafault para a dura��o
        if (pgm_read_byte(&musica2[i + 1]) == '#')
        {
            switch (pgm_read_byte(&musica2[i]))
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
            switch (pgm_read_byte(&musica2[i]))
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

        if (pgm_read_byte(&musica2[i]) == '.')
        {
            n = n + (n / 2); // dura��o 50% >
            i++;
        }
        if (pgm_read_byte(&musica2[i]) == '4')
        {
            o = 4;
            i++;
        }
        else if (pgm_read_byte(&musica2[i]) == '5')
        {
            o = 5;
            i++;
        }
        else if (pgm_read_byte(&musica2[i]) == '6')
        {
            o = 6;
            i++;
        }
        else if (pgm_read_byte(&musica2[i]) == '7')
        {
            o = 7;
            i++;
        }
        if (pgm_read_byte(&musica2[i]) == '.')
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
        set_bit(TCCR1A, COM1A0);                   // habilita o som
    } while (pgm_read_byte(&musica2[i++]) == ','); // leitura at� o final da m�sica
    TCCR1A = 0;                                    // desabilita o som e o TC1
    TCCR1B = 0;
}