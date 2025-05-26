#include "utils.c"
#include <lcom/lcf.h>
#include <stdint.h>
#include <stdio.h>
#include "font.h"

vbe_mode_info_t mode_info;
uint8_t *frame_buffer;
extern const uint8_t font8x8_basic[128][8];

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

void draw_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b){
  if(x >= mode_info.XResolution || y >= mode_info.YResolution) return ;
  unsigned i = (y *  mode_info.XResolution + x) * 3;
  frame_buffer[i] = b;
  frame_buffer[i + 1] = g;
  frame_buffer[i + 2] = r;
}

void draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t heigh, uint8_t r, uint8_t g, uint8_t b){
  for(unsigned i = 0; i < heigh; i++){
    for(unsigned j = 0; j < width; j++){
      draw_pixel(x + j, y+i, r, g, b);
    }
  }
}

Letter letters[256];
int letters_count = 0;

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

void redraw_letter(int index, int size) {
    if (index < 0 || index >= letters_count) return;
    Letter *letter = &letters[index];
    draw_char(letter->x, letter->y, letter->c, size, letter->r, letter->g, letter->b);
}
