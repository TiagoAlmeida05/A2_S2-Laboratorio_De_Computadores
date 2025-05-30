#include "mouse.h"
#include "timer.c"

int mouseHookId = MOUSE_IRQ;
uint8_t packetBytes[3];
uint8_t byteIndex = 0;

mouse_packet packet;

int (mouse_subscribe_int)(uint8_t *bit_no){
    *bit_no = mouseHookId;
    return sys_irqrmpolicy(MOUSE_IRQ,IRQ_REENABLE|IRQ_EXCLUSIVE,&mouseHookId);
}

int(mouse_unsubscribe_int)(){
    return sys_irqrmpolicy(&mouseHookId);
}

void(mouse_process_packet)(){
    packet.lb = packet.bytes[0] & LEFT_BUTTON;
    packet.mb = packet.bytes[0] & MIDDLE_BUTTON;
    packet.rb = packet.bytes[0] & RIGHT_BUTTON;
    packet.x_ov = packet.bytes[0] & BIT(6);
    packet.y_ov = packet.bytes[0] & BIT(7);
    packet.delta_x = (packet.bytes[0] & BIT(4)) ? (int16_t)(packet.bytes[1] | 0xFF00) : packet.bytes[1];
    packet.delta_y = (packet.bytes[0] & BIT(5)) ? (int16_t)(packet.bytes[2] | 0xFF00) : packet.bytes[2];
    packet.delta_y = -packet.delta_y;
}

void (mouse_ih)(){
    uint8_t value;
    if(util_sys_inb(MOUSE_CMD_REG, &value)!= OK){
        return;
    }
    if((value & MOUSE_OBF)&& !(value & (MOUSE_PARITY|MOUSE_PARITY))){
        uint8_t data;
        if(util_sys_inb(MOUSE_OUT_BUF,&data)!= OK){
            return;
        }
        if(byteIndex == 0 && (data & FIRST_BYTE_VALID)){
            return;
        }
        packetBytes[byteIndex++] = data;

        if(byteIndex == 3){
            for(int i = 0; i<3;++i){
                packet.bytes[i] = packetBytes[i];
            }
            mouse_process_packet();
            byteIndex = 0
        }
    }
}
int (mouse_write_command)(uint8_t command) {
    uint8_t status;
    for (int i = 0; i < 5; i++) {
        if (util_sys_inb(MOUSE_CMD_REG, &status) != OK){
            continue;
        } 
        if ((status & BIT(1)) == 0) {
            if (sys_outb(MOUSE_CMD_REG, WRITE_BYTE_TO_MOUSE) != OK){
                return 1;
            } 
            if (sys_outb(MOUSE_OUT_BUF, command) != OK){
                return 1;
            } 
            return 0;
        }
        wait_ms(20);
    }

    return 1;
}

