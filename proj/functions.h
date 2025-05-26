#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <lcom/lcf.h>

extern int current_letter;
extern uint8_t scancode;
extern uint8_t irq_set;
void process_key(char key, int size);
void draw_menu();
void draw_easy(int size);


#endif
