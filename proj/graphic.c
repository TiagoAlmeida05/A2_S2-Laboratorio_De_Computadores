#include "utils.c"
#include <lcom/lcf.h>
#include <stdint.h>
#include <stdio.h>
#include "font.h"
#include "functions.h"


vbe_mode_info_t mode_info;   /** @brief Informações do modo gráfico atual */
uint8_t *frame_buffer;       /** @brief Ponteiro para o frame buffer */
extern const uint8_t font8x8_basic[128][8];   /** @brief Fonte básica 8x8 */

/**
 * @brief Define o modo gráfico usando a BIOS (VBE).
 * 
 * @param mode Modo gráfico a ser definido (ex: 0x117).
 * @return int 0 em caso de sucesso, 1 em caso de erro.
 */

int set_graphic_mode(uint16_t mode){
  reg86_t reg86;
  memset(&reg86, 0, sizeof(reg86));
  reg86.intno = 0x10;
  reg86.al = 0x02;
  reg86.ah = 0x4F;
  reg86.bx = mode | BIT(14);
  if(sys_int86(&reg86) != 0){
    return 1;
  }
  return 0;
}

/**
 * @brief Mapeia o frame buffer do modo gráfico para a memória do processo.
 * 
 * @param mode Modo gráfico atual.
 * @return int 0 em caso de sucesso, 1 em caso de erro.
 */

int set_frame_buffer(uint16_t mode){
  memset(&mode_info, 0, sizeof(mode_info));
  if(vbe_get_mode_info(mode, &mode_info) != 0) return 1;

  unsigned int bytes_per_pixel = (mode_info.BitsPerPixel + 7) /8;
  unsigned int frame_size = mode_info.XResolution * mode_info.YResolution * bytes_per_pixel;

  struct minix_mem_range p_address;
  p_address.mr_base = mode_info.PhysBasePtr;
  p_address.mr_limit = p_address.mr_base + frame_size;

  if(sys_privctl(SELF, SYS_PRIV_ADD_MEM, &p_address) != 0) return 1;

  frame_buffer = vm_map_phys(SELF, (void*) p_address.mr_base, frame_size);
  if(frame_buffer == NULL) return 1;
  return 0;
}

/**
 * @brief Pinta o ecrã com uma cor RGB.
 * 
 * @param r Valor do vermelho (0-255)
 * @param g Valor do verde (0-255)
 * @param b Valor do azul (0-255)
 */

void paint_screen(uint8_t r, uint8_t g, uint8_t b){

  for(unsigned y = 0; y < mode_info.YResolution; y++){
    for(unsigned x = 0; x < mode_info.XResolution; x++){
      unsigned i = (y *  mode_info.XResolution + x) * 3;
      frame_buffer[i] = b;
      frame_buffer[i + 1] = g;
      frame_buffer[i + 2] = r;
    }
  }
}

/**
 * @brief Desenha um pixel no ecrã.
 * 
 * @param x Coordenada horizontal
 * @param y Coordenada vertical
 * @param r Valor do vermelho (0-255)
 * @param g Valor do verde (0-255)
 * @param b Valor do azul (0-255)
 */

void draw_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b){
  if(x >= mode_info.XResolution || y >= mode_info.YResolution) return ;
  unsigned i = (y *  mode_info.XResolution + x) * 3;
  frame_buffer[i] = b;
  frame_buffer[i + 1] = g;
  frame_buffer[i + 2] = r;
}

/**
 * @brief Desenha um retângulo colorido no ecrã.
 * 
 * @param x Coordenada X do canto superior esquerdo
 * @param y Coordenada Y do canto superior esquerdo
 * @param width Largura do retângulo
 * @param heigh Altura do retângulo
 * @param r Componente vermelha da cor
 * @param g Componente verde da cor
 * @param b Componente azul da cor
 */

void draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t heigh, uint8_t r, uint8_t g, uint8_t b){
  for(unsigned i = 0; i < heigh; i++){
    for(unsigned j = 0; j < width; j++){
      draw_pixel(x + j, y+i, r, g, b);
    }
  }
}

Letter letters[256];    /**< Array de letras desenhadas na tela */
int letters_count = 0;  /**< Número total de letras desenhadas */

/**
 * @brief Desenha um caractere no ecrã usando a fonte 8x8.
 * 
 * @param x Coordenada X
 * @param y Coordenada Y
 * @param c Caractere a ser desenhado
 * @param size Fator de escala do caractere
 * @param r Componente vermelha da cor
 * @param g Componente verde da cor
 * @param b Componente azul da cor
 */

void draw_char(uint16_t x, uint16_t y, char c, int size, uint8_t r, uint8_t g, uint8_t b) {
  uint8_t *bitmap = (uint8_t *)font8x8_basic[(int)c];
  for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 8; j++) {
          if (bitmap[i] & (1 << (7 - j))) {
              for (int dy = 0; dy < size; dy++) {
                  for (int dx = 0; dx < size; dx++) {
                      draw_pixel(x + j * size + dx, y + i * size + dy, r, g, b);
                  }
              }
          }
      }
  }
}

/**
 * @brief Desenha uma string de texto no ecrã, com mudança automática de linha, caso seja necessário.
 * 
 * @param x Coordenada X inicial
 * @param y Coordenada Y inicial
 * @param str Texto a ser desenhado
 * @param size Tamanho da fonte
 * @param r Cor vermelha
 * @param g Cor verde
 * @param b Cor azul
 */

void draw_string(uint16_t x, uint16_t y, const char* str, int size, uint8_t r, uint8_t g, uint8_t b){
  int i = 0;
  int baseX = x;
  int char_w = 8*size;
  int char_h = 8*size;
  letters_count = 0;

  while(str[i] != '\0'){

    int word_l = 0;

    while(str[i + word_l] != ' ' && str[i + word_l] != '\0'){
      word_l++;
    }

    int word_w = word_l* char_w;

    if(x + word_w >= mode_info.XResolution){
      y += char_h + 10;
      x = baseX;
    }

    for(int j = 0; j <word_l; j++){
      draw_char(x, y,str[i], size, r, g, b);

      letters[letters_count].x = x;
      letters[letters_count].y = y;
      letters[letters_count].c = str[i];
      letters[letters_count].r = 255;
      letters[letters_count].g = 255;
      letters[letters_count].b = 255;
      letters_count++;

      x += 8 * size;  
      i++;
    }

    if(str[i] == ' '){
      x += char_w;
      i++;
    }
  }
}

/**
 * @brief Redesenha uma letra específica no ecrã.
 * 
 * @param index Índice da letra no array `letters`
 * @param size Tamanho da fonte
 */

void redraw_letter(int index, int size) {
    if (index < 0 || index >= letters_count) return;
    Letter *letter = &letters[index];
    draw_char(letter->x, letter->y, letter->c, size, letter->r, letter->g, letter->b);
}

/**
 * @brief Desenha o tempo centrado no ecrã.
 * 
 * @param y Coordenada Y para desenhar o tempo
 * @param counter Contador de "ticks" (assumindo 60 por segundo)
 * @param size Tamanho da fonte
 * @param r Cor vermelha
 * @param g Cor verde
 * @param b Cor azul
 */

void draw_time_centered(uint16_t y, int counter, int size, uint8_t r, uint8_t g, uint8_t b) {
    int seconds = counter / 60;
    int milliseconds = (counter % 60) * 1000 / 60;

    char time_str[32];
    sprintf(time_str, "TIME: %d.%03d s", seconds, milliseconds);

    int time_width = string_pixel_width(time_str, size);
    int x = (mode_info.XResolution - time_width) / 2;

    int i = 0;
    while (time_str[i] != '\0') {
        draw_char(x, y, time_str[i], size, r, g, b);
        x += 8 * size;
        i++;
    }
}
