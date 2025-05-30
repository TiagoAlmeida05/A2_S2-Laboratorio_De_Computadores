#ifndef MOUSE_H
#define MOUSE_H

#include <lcom/lcf.h>


#define MOUSE_OUT_BUF 0x60
#define MOUSE_CMD_REG 0x64
#define WRITE_BYTE_TO_MOUSE 0xD4     
#define ENABLE_DATA_REPORTING 0xF4   
#define DISABLE_DATA_REPORTING 0xF5  

#define MOUSE_OBF BIT(0)     
#define MOUSE_PARITY BIT(7)  
#define MOUSE_TIMEOUT BIT(6) 
#define LEFT_BUTTON     BIT(0)
#define RIGHT_BUTTON    BIT(1)
#define MIDDLE_BUTTON   BIT(2)
#define FIRST_BYTE_VALID BIT(3) 

#define MOUSE_IRQ 12 


struct mouse_packet {
  uint8_t bytes[3];       
  bool lb, mb, rb;        
  int16_t delta_x, delta_y; 
  bool x_ov, y_ov;       
};


int (mouse_subscribe_int)(uint8_t *bit_no);

int (mouse_unsubscribe_int)();

void (mouse_ih)();

int (mouse_write_command)(uint8_t command);

void (mouse_process_packet)();

extern struct mouse_packet packet;

#endif /* MOUSE_H */
