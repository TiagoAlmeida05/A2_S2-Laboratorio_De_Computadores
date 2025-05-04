#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"
int counter = 0;
int(timer_set_frequency)(uint8_t timer, uint32_t freq) {
  if (freq < 19 || freq > TIMER_FREQ) {
    printf("Invalid timer or frequency\n");
    return 1;
  }

  uint16_t counter = TIMER_FREQ / freq;
  uint8_t lsb, msb;

  if (util_get_LSB(counter, &lsb) != 0 || util_get_MSB(counter, &msb) != 0) {
    return 1;
  }

  uint8_t st;
  if (timer_get_conf(timer, &st) != 0) {
    return 1;
  }

  st = (st & (TIMER_BCD | TIMER_SQR_WAVE | BIT(3))) | TIMER_LSB_MSB;

  if (sys_outb(TIMER_CTRL, st) != 0) {
    return 1;
  }

  uint16_t port = (timer == 0) ? TIMER_0 : (timer == 1) ? TIMER_1
                                                        : TIMER_2;

  if (sys_outb(port, lsb) != 0 || sys_outb(port, msb) != 0) {
    return 1;
  }

  return 0;
}

int hook_id = 0;
int(timer_subscribe_int)(uint8_t *bit_no) {
  *bit_no = BIT(hook_id);
  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) != 0) {
    return 1;
  }
  return 0;
}

int(timer_unsubscribe_int)() {
  if (sys_irqrmpolicy(&hook_id) != 0) {
    return 1;
  }

  return 0;
}

void(timer_int_handler)() {
  counter++;
}

int(timer_get_conf)(uint8_t timer, uint8_t *st) {
  if(st == NULL || timer > 2 || timer < 0) return 1;
  uint8_t cmd = (TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer));
  if (sys_outb(TIMER_CTRL, cmd) != 0) {
    return 1;
  }
  if(util_sys_inb(TIMER_0 + timer, st)) return 1;
  return 0;
}

int(timer_display_conf)(uint8_t timer, uint8_t conf, enum timer_status_field field) {
  union timer_status_field_val data;

  switch (field) {
    case tsf_all: 
        data.byte = conf; 
        break;

    case tsf_initial: 
        conf = (conf >> 4) & 0x03;

        if(conf== 1) data.in_mode = LSB_only;
        else if(conf== 2) data.in_mode = MSB_only;
        else if(conf== 3) data.in_mode = MSB_after_LSB;
        else data.in_mode = INVAL_val;
        break;

    case tsf_mode: 
        conf = (conf >> 1) & 0x07;

        if(conf == 6) data.count_mode = 2;
        else if(conf == 7) data.count_mode = 3;
        else data.count_mode = conf;
  
        break;

    case tsf_base:
        data.bcd = conf & TIMER_BCD;
        break;        
  
    default:
        return 1;
    }
  
    if (timer_print_config(timer, field, data) != 0) return 1;
    return 0;
}

