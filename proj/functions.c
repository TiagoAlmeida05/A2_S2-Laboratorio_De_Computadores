#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>
#include "font.h"
#include "functions.h"
#include "graphic.h"

void process_key(char key, int size) {
    if (current_letter >= letters_count) return;

    if (letters[current_letter].c == key) {
        letters[current_letter].r = 0;
        letters[current_letter].g = 255;
        letters[current_letter].b = 0;
        redraw_letter(current_letter, size);
        current_letter++;
    }
}

void draw_menu(){
  paint_screen(64, 64, 64);
  draw_exit();
  draw_string(80, 125, "SPEED TEST", 8, 255, 255, 255);
  draw_rectangle(80, 200, 640, 10, 255, 255, 255);
  draw_rectangle(310, 290, 180, 60, 0, 0, 0);
  draw_rectangle(270, 365, 260, 60, 0, 0, 0);
  draw_rectangle(310, 440, 180, 60, 0, 0, 0);
  draw_rectangle(315, 295, 170, 50, 0, 255, 0);
  draw_rectangle(275, 370, 250, 50, 255, 255, 0);
  draw_rectangle(315, 445, 170, 50, 255, 0, 0);
  draw_string(320, 300, "EASY", 5, 0, 0, 0);
  draw_string(280, 375, "MEDIUM", 5, 0, 0, 0);
  draw_string(320, 450, "HARD", 5, 0, 0, 0);
}

void draw_easy(int size){
  int easy = rand() % 5;

  paint_screen(64, 64, 64);
  draw_exit();

  draw_string(20, 125, easy_sentences[easy], size, 255, 255, 255);

  current_letter = 0;
}

void draw_medium(int size){
  int medium = rand() % 5;

  paint_screen(64, 64, 64);
  draw_exit();

  draw_string(20, 125, medium_sentences[medium], size, 255, 255, 255);

  current_letter = 0;
}

void draw_hard(int size){
  int hard = rand() % 5;

  paint_screen(64, 64, 64);
  draw_exit();

  draw_string(20, 125, hard_sentences[hard], size, 255, 255, 255);

  current_letter = 0;
}

void draw_exit(){
  draw_rectangle(0, 0, 104, 35, 0, 0, 0);
  draw_string(5,5,"EXIT", 3, 255, 255, 255);
}

int string_pixel_width(const char* str, int size) {
  int len = strlen(str);
  return len * 8 * size;
}

void draw_mouse() {
    uint16_t x = mode_info.XResolution / 2;
    uint16_t y = mode_info.YResolution / 2;

    uint8_t r = 255, g = 255, b = 255;
    int size = 10;

    
    for (int i = -size; i <= size; i++) {
        draw_pixel(x + i, y, r, g, b);
    }

    
    for (int i = -size; i <= size; i++) {
        draw_pixel(x, y + i, r, g, b);
    }
}
