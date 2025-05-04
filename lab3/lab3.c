#include "kbd.h"
#include "timer.c"
#include <lcom/lab3.h>
#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>

extern uint8_t scancode;
int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

uint32_t inb_counter = 0;

int(kbd_test_scan)() {
  int ipc_status;
  message msg;
  int r;
  uint8_t irq_set;

  if (keyboard_subscribe_int(&irq_set) != 0) {
    return 1;
  }

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
            if (scancode & BIT(7)) {
              kbd_print_scancode(false, scancode == 0xE0 ? 2 : 1, &scancode);
            }
            else {
              kbd_print_scancode(true, scancode == 0xE0 ? 2 : 1, &scancode);
            }
          }
      }
    }
  }
  if (kbd_print_no_sysinb(inb_counter) != 0)
    return 1;
  if (keyboard_unsubscribe_int() != 0)
    return 1;
  return 0;
}

int(kbd_test_poll)() {
  /* To be completed by the students */
  printf("%s is not yet implemented!\n", __func__);

  return 1;
}

extern int counter;

int(kbd_test_timed_scan)(uint8_t n) {
  int ipc_status;
  message msg;
  int r;
  uint8_t irqk_set;
  uint8_t irqt_set;
  uint8_t time = n;

  if (keyboard_subscribe_int(&irqk_set) != 0) {
    return 1;
  }
  if (timer_subscribe_int(&irqt_set) != 0) {
    return 1;
  }

  while (scancode != 0x81 && time > 0) {
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) {
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irqk_set) {
            time = n;
            counter = 0;
            kbc_ih();
            if (scancode & BIT(7)) {
              kbd_print_scancode(false, scancode == 0xE0 ? 2 : 1, &scancode);
            }
            else {
              kbd_print_scancode(true, scancode == 0xE0 ? 2 : 1, &scancode);
            }
          }
          if (msg.m_notify.interrupts & irqt_set) {
            timer_int_handler();
            if(counter % sys_hz() == 0){
              time--;
            }
          }
      }
    }
  }
  if (keyboard_unsubscribe_int() != 0)
    return 1;
  if (kbd_print_no_sysinb(inb_counter) != 0)
    return 1;
  if(timer_unsubscribe_int() != 0)
    return 1;
  return 0;
}
