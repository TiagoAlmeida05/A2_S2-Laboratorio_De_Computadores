#include <lcom/lcf.h>
#include <lcom/lab3.h>
#include <stdbool.h>
#include <stdint.h>
#include "kbd.h"
#include "utils.c"

uint8_t scancode;
int keyboard_hook_id = 1;

void (kbc_ih)() {
  uint8_t value;
  if(util_sys_inb(KBD_STAT_REG, &value) != 0){
    return ;
  }
  if(!(value & KBD_OBF)){
    return ;
  }
  if(value&(KBD_PAR_ERR | KBD_TO_ERR)){
    return ;
  }
  if(util_sys_inb(KBD_OUT_BUF, &scancode) != 0){
    return ;
  }
}

int (keyboard_subscribe_int) (uint8_t *bit_no){
  if(bit_no == NULL) return 1;
  *bit_no = BIT(keyboard_hook_id);
  return sys_irqsetpolicy(KEYBOARD_IRQ, IRQ_REENABLE|IRQ_EXCLUSIVE, &keyboard_hook_id);
}

int keyboard_unsubscribe_int(){
  return sys_irqrmpolicy(&keyboard_hook_id);
}
