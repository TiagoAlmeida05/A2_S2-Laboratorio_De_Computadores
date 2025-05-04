#ifndef GRAPHIC_H
#define GRAPHIC_H

#include <lcom/lcf.h>

int set_graphic_mode(uint16_t mode);
int set_frame_buffer(uint16_t mode);
void paint_screen(uint8_t r, uint8_t g, uint8_t b);
void draw_pixel(uint16_t x, uint16_t y,uint8_t r, uint8_t g, uint8_t b);
void draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t heigh, uint8_t r, uint8_t g, uint8_t b);
void draw_char(uint16_t x, uint16_t y, char c, uint8_t r, uint8_t g, uint8_t b);
void draw_string(uint16_t x, uint16_t y, const char* str, uint8_t r, uint8_t g, uint8_t b);
#endif // KBD_H
