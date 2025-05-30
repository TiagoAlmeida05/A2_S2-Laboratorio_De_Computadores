#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <lcom/lcf.h>
#include <lcom/vbe.h>

extern vbe_mode_info_t mode_info;
extern int current_letter;
extern uint8_t scancode;
extern uint8_t irq_set;
void process_key(char key, int size);
void draw_mouse(void);
void draw_menu();
void draw_easy(int size);
void draw_medium(int size);
void draw_hard(int size);
void draw_exit();
int string_pixel_width(const char* str, int size);

#endif
