#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>
#include "graphic.h"
#include "kbd.h"

int main(int argc, char *argv[]) {
    // sets the language of LCF messages (can be either EN-US or PT-PT)
    lcf_set_language("EN-US");
  
    // enables to log function invocations that are being "wrapped" by LCF
    // [comment this out if you don't want/need it]
    lcf_trace_calls("/home/lcom/labs/proj/trace.txt");
  
    // enables to save the output of printf function calls on a file
    // [comment this out if you don't want/need it]
    lcf_log_output("/home/lcom/labs/proj/output.txt");
  
    // handles control over to LCF
    // [LCF handles command line arguments and invokes the right function]
    if (lcf_start(argc, argv))
      return 1;
  
    // LCF clean up tasks
    // [must be the last statement before return]
    lcf_cleanup();
  
    return 0;
  }

extern uint8_t scancode;

int (proj_main_loop)(int argc, char* argv[]) {
  int ipc_status;
  message msg;
  int r;
  uint8_t irq_set;
  
  if (keyboard_subscribe_int(&irq_set) != 0) {
    return 1;
  }

  set_graphic_mode(0x115);
  set_frame_buffer(0x115);
  paint_screen(64, 64, 64);
  draw_rectangle(0, 0, 104, 35, 0, 0, 0);
  draw_string(5,5,"EXIT", 255, 255, 255);

  while (scancode != 0x81) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) {
            kbc_ih();
          }
      }
    }
  }
  printf("Off the game");
  if (keyboard_unsubscribe_int() != 0)
    return 1;
  vg_exit();
return 0;
}
