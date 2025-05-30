#include <lcom/lcf.h>
#include <stdbool.h>
#include <stdint.h>
#include "kbd.h"

uint8_t scancode;   /** @brief Armazena o último scancode lido */
int keyboard_hook_id = 1;   /** @brief ID do hook do teclado para a subscrição de interrupções */


/**
 * @brief Handler da interrupção do teclado (Interrupt Handler).
 *
 * Lê o byte de estado do controlador de teclado. Se houver erro de paridade ou timeout,
 * ou se o buffer de saída não estiver cheio, a função retorna sem fazer nada.
 * Caso contrário, lê o scancode do buffer de saída e armazena na variável global `scancode`.
 */

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

/**
 * @brief Subscreve as interrupções do teclado.
 *
 * Armazena o número do bit correspondente ao ID da interrupção no ponteiro fornecido.
 * Define a política de interrupção para o teclado.
 *
 * @param bit_no Ponteiro para onde o número do bit será armazenado.
 * @return 0 em caso de sucesso, 1 em caso de erro.
 */

int (keyboard_subscribe_int) (uint8_t *bit_no){
  if(bit_no == NULL) return 1;
  *bit_no = BIT(keyboard_hook_id);
  return sys_irqsetpolicy(KEYBOARD_IRQ, IRQ_REENABLE|IRQ_EXCLUSIVE, &keyboard_hook_id);
}

/**
 * @brief Cancela a subscrição das interrupções do teclado.
 *
 * @return 0 em caso de sucesso, valor diferente de 0 em caso de erro.
 */
 
int (keyboard_unsubscribe_int)(){
  return sys_irqrmpolicy(&keyboard_hook_id);
}
