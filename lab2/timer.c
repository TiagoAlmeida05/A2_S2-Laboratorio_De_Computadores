#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"
int counter = 0;
int (timer_set_frequency)(uint8_t timer, uint32_t freq) {
  if (timer > 2 || freq == 0 || freq > TIMER_FREQ) {
    printf("Invalid timer or frequency\n");
    return 1;
}

uint16_t counter = TIMER_FREQ / freq;
uint8_t lsb, msb;

if (util_get_LSB(counter, &lsb) != 0 || util_get_MSB(counter, &msb) != 0) {
    printf("Error extracting LSB/MSB\n");
    return 1;  
}

uint8_t st;
if (timer_get_conf(timer, &st) != 0) {
    printf("Failed to read timer configuration\n");
    return 1;
}

uint8_t control_word = (st & 0x0F) | TIMER_LSB_MSB | (timer == 0 ? TIMER_SEL0 : (timer == 1 ? TIMER_SEL1 : TIMER_SEL2));

if (sys_outb(TIMER_CTRL, control_word) != 0) {
    printf("Failed to write control word\n");
    return 1;
}

uint16_t port = (timer == 0) ? TIMER_0 : (timer == 1) ? TIMER_1 : TIMER_2;

if (sys_outb(port, lsb) != 0 || sys_outb(port, msb) != 0) {
    printf("Failed to write LSB/MSB\n");
    return 1;
}

return 0;
}

int hook_id = 0;
int (timer_subscribe_int)(uint8_t *bit_no) {
  *bit_no = BIT(hook_id);
    if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) != 0) {
        return 1; 
    }   
    return 0;
}

int (timer_unsubscribe_int)() {
  if (sys_irqrmpolicy(&hook_id) != 0) {
    return 1;
}

return 0;
}

void (timer_int_handler)() {
   counter++;
}

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  if (timer > 2 || st == NULL) {
    return 1;
}

uint8_t cmd = TIMER_RB_CMD | TIMER_RB_STATUS_ | TIMER_RB_SEL(timer);

if (sys_outb(TIMER_CTRL, cmd) != 0) {
    return 1;  
}

uint16_t port = (timer == 0) ? TIMER_0 : (timer == 1) ? TIMER_1 : TIMER_2;

util_sys_inb(port, st);

return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t conf, enum timer_status_field field) {
  printf("Timer %d Configuration:\n", timer);

    switch (field) {
        case tsf_initial: {
            uint8_t access_type = (conf >> 4) & 0x03;
            printf("  Access Type: ");
            switch (access_type) {
                case 0: printf("Latch Count Value Command\n"); break;
                case 1: printf("LSB only\n"); break;
                case 2: printf("MSB only\n"); break;
                case 3: printf("LSB followed by MSB\n"); break;
                default: printf("Unknown\n"); break;
            }
            break;
        }

        case tsf_mode: {
            uint8_t mode = (conf >> 1) & 0x07;
            printf("  Mode: ");
            switch (mode) {
                case 0: printf("Interrupt on Terminal Count (Mode 0)\n"); break;
                case 1: printf("Hardware Retriggerable One-shot (Mode 1)\n"); break;
                case 2: printf("Rate Generator (Mode 2)\n"); break;
                case 3: printf("Square Wave Generator (Mode 3)\n"); break;
                case 4: printf("Software Triggered Strobe (Mode 4)\n"); break;
                case 5: printf("Hardware Triggered Strobe (Mode 5)\n"); break;
                default: printf("Unknown Mode\n"); break;
            }
            break;
        }

        case tsf_base: {
            uint8_t base = conf & TIMER_BCD;
            printf("  Counting Mode: %s\n", base ? "BCD (Binary Coded Decimal mode)" : "Binary (16-bit mode)");
            break;
        }

        case tsf_all: {
            timer_display_conf(timer, conf, tsf_initial);
            timer_display_conf(timer, conf, tsf_mode);
            timer_display_conf(timer, conf, tsf_base);
            break;
        }

        default:
            printf("Invalid field\n");
            return 1;
    }

    return 0; 
}

