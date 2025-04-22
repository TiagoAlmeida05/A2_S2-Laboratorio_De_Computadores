#ifndef KBD_H
#define KBD_H

#include <lcom/lcf.h>

#define KBD_OUT_BUF 0x60
#define KBD_STAT_REG 0x64
#define KBC_CMD_REG 0x64   
#define KBD_OUT_BUF 0x60   
#define KBC_READ_CMD 0x20   
#define KBC_WRITE_CMD 0x60

#define KBD_OBF BIT(0)
#define KBD_PAR_ERR BIT(7)
#define KBD_TO_ERR BIT(6)

#define KEYBOARD_IRQ 1

int (keyboard_subscribe_int)(uint8_t *bit_no);
int (keyboard_unsubscribe_int)();
void (kbc_ih)();
int (kbd_enable_interrupts)();
int (kbd_disable_interrupts)();

extern uint8_t scancode;
extern bool two_byte;

#endif // KBD_H
