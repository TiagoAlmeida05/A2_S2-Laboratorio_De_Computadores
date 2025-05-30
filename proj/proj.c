#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>
#include "graphic.h"
#include "kbd.h"
#include "font.h"
#include "functions.h"
#include "timer.c"

/**
 * @brief Função principal da aplicação baseada no LCF.
 * 
 * Define configurações do LCF e entrega o controlo ao framework.
 * 
 * @param argc Número de argumentos.
 * @param argv Vetor de argumentos.
 * @return int Retorna 0 em caso de sucesso, 1 em caso de erro.
 */

int main(int argc, char *argv[]) {
    
    lcf_set_language("EN-US");
  
    lcf_trace_calls("/home/lcom/labs/proj/trace.txt");
  
    lcf_log_output("/home/lcom/labs/proj/output.txt");
  
    if (lcf_start(argc, argv))
      return 1;
  
    lcf_cleanup();
  
    return 0;
  }

// Variáveis externas usadas entre módulos
extern uint8_t scancode;
extern int counter;
int current_letter = 0;
extern vbe_mode_info_t mode_info;

/**
 * @brief Ciclo principal do projeto executado após a inicialização pelo LCF.
 * 
 * Garante o tratamento das interrupções, lógica dos estados do jogo, desenho do ecrã e processamento do teclado.
 * 
 * @param argc Número de argumentos.
 * @param argv Vetor de argumentos.
 * @return int Retorna 0 em caso de sucesso, valor diferente de 0 em caso de erro.
 */

int (proj_main_loop)(int argc, char* argv[]) {
  int ipc_status;
  message msg;
  int r;
  uint8_t irq_seth, irq_sett;
  bool running = true;

  if (keyboard_subscribe_int(&irq_seth) != 0)
    return 1;
  if (timer_subscribe_int(&irq_sett) != 0)
    return 1;

  set_graphic_mode(0x115);
  set_frame_buffer(0x115);
  timer_set_frequency(0, 60);

  int game_state = 0;
  bool screen_drawn = false;
  counter = 0;          
  int final_time_counter = 0; 

  draw_menu();
  draw_mouse();
  screen_drawn = true;

  while (running) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d\n", r);
      continue;
    }

    if (!is_ipc_notify(ipc_status)) 
      continue;

    // Interrupção do teclado

    if (msg.m_notify.interrupts & irq_seth) {
      kbc_ih();
      if (scancode == 0) continue;

      char key = scancode_to_char[scancode];
      if (key >= 'a' && key <= 'z') 
        key = key - 'a' + 'A';

      // Gestão dos estados do jogo

      switch (game_state) {
        case 0:
          if (!screen_drawn) {
            draw_menu();
            screen_drawn = true;
          }
          if (key == 'E') {
            game_state = 1;
            screen_drawn = false;
          }
          else if (key == 'M') {
            game_state = 2;
            screen_drawn = false;
          }
          else if (key == 'H') {
            game_state = 3;
            screen_drawn = false;
          }
          else if (key == 'Q') {
            running = false;
          }
          break;

        case 1:
          if (!screen_drawn) {
            draw_easy(5);
            counter = 0;
            screen_drawn = true;
            game_state = 4;
          }
          break;

        case 2:
          if (!screen_drawn) {
            draw_medium(5);
            counter = 0;
            screen_drawn = true;
            game_state = 4;
          }
          break;

        case 3:
          if (!screen_drawn) {
            draw_hard(5);
            counter = 0;
            screen_drawn = true;
            game_state = 4;
          }
          break;

        case 4: 
          if (scancode == 0x81) { 
            game_state = 0;
            screen_drawn = false;
          } else if (key != 0) {
            process_key(key, 5);
            if (current_letter >= letters_count) {
              final_time_counter = counter; 
              game_state = 5;               
              screen_drawn = false;
            }
          }
          break;

        case 5: 
          if (scancode == 0x1C) { 
            game_state = 0;
            screen_drawn = false;
          }
          break;
      }

      scancode = 0;
    }

     // Interrupção do temporizador
     
    if (msg.m_notify.interrupts & irq_sett) {
      timer_int_handler();

      if (game_state == 4) {
        draw_rectangle(150, 20, mode_info.XResolution - 150, 30, 64, 64, 64);
        draw_time_centered(20, counter, 2, 255, 255, 255);
      }else if (game_state == 5 && !screen_drawn) {
        draw_rectangle(0, mode_info.YResolution / 2 - 40, mode_info.XResolution, 150, 0, 0, 0);

        draw_time_centered(mode_info.YResolution / 2 - 10, final_time_counter, 3, 255, 255, 255);

        const char* msg = "PRESS ENTER TO GO TO THE MENU";
        int msg_width = string_pixel_width(msg, 2);
        int msg_x = (mode_info.XResolution - msg_width) / 2;
        draw_string(msg_x, mode_info.YResolution / 2 + 50, msg, 2, 255, 255, 255);

        screen_drawn = true;
      }
    }
  }

  keyboard_unsubscribe_int();
  timer_unsubscribe_int();
  vg_exit();
  return 0;
}
