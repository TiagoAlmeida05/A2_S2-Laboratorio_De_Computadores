#include "mouse.h"
#include "timer.c"



int mouseHookId = MOUSE_IRQ;    /** @brief ID usado para o hook de interrupção do mouse */
uint8_t packetBytes[3];         /** @brief Bytes temporários armazenados da leitura do pacote do mouse */
uint8_t byteIndex = 0;          /** @brief Índice do byte atual sendo lido do pacote do mouse */

struct mouse_packet packet;     /** @brief Estrutura que contém o pacote final interpretado do mouse */

/**
 * @brief Subscreve as interrupções do mouse.
 *
 * @param bit_no Ponteiro para armazenar o número do bit correspondente ao hook do mouse.
 * @return 0 em caso de sucesso, diferente de 0 em caso de erro.
 */


int (mouse_subscribe_int)(uint8_t *bit_no){
    *bit_no = mouseHookId;
    return sys_irqsetpolicy(MOUSE_IRQ,IRQ_REENABLE|IRQ_EXCLUSIVE,&mouseHookId);
}

 /**
 * @brief Cancela a subscrição das interrupções do mouse.
 *
 * @return 0 em caso de sucesso, diferente de 0 em caso de erro.
 */
 
int(mouse_unsubscribe_int)(){
    return sys_irqrmpolicy(&mouseHookId);
}

/**
 * @brief Processa os 3 bytes do pacote do mouse e enche a estrutura `packet`.
 *
 * Interpreta os sinais de botões e deltas de movimento com sinal.
 */

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

/**
 * @brief Handler da interrupção do mouse.
 *
 * Lê bytes do buffer de saída e monta um pacote completo do mouse a cada 3 bytes.
 * Quando completo, processa o pacote.
 */

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
            byteIndex = 0;
        }
    }
}

/**
 * @brief Envia um comando ao mouse.
 *
 * Tenta até 5 vezes enviar o comando usando os registradores apropriados.
 *
 * @param command Comando a ser enviado ao mouse.
 * @return 0 em caso de sucesso, 1 se falhar após todas as tentativas.
 */

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
