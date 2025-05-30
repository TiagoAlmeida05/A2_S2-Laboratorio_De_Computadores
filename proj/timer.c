#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"


int counter = 0;    /**  @brief Contador global de interrupções do timer */


/**
 * @brief Configura a frequência de operação de um dos timers do i8254.
 *
 * @param timer Timer a configurar (0, 1 ou 2).
 * @param freq Frequência desejada (deve ser maior que 18 Hz e menor que TIMER_FREQ).
 * @return 0 em caso de sucesso, 1 em caso de erro.
 */

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

int hook_id = 0;    /** ID do hook para interrupções do timer*/

/**
 * @brief Subscreve as interrupções do timer.
 *
 * @param bit_no Ponteiro para armazenar o número do bit correspondente ao hook_id.
 * @return 0 em caso de sucesso, 1 em caso de erro.
 */

int(timer_subscribe_int)(uint8_t *bit_no) {
  *bit_no = BIT(hook_id);
  if (sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) != 0) {
    return 1;
  }
  return 0;
}

/**
 * @brief Cancela a subscrição das interrupções do timer.
 *
 * @return 0 em caso de sucesso, 1 em caso de erro.
 */

int(timer_unsubscribe_int)() {
  if (sys_irqrmpolicy(&hook_id) != 0) {
    return 1;
  }

  return 0;
}

/**
 * @brief Handler da interrupção do timer.
 *
 * Incrementa a variável global `counter` a cada interrupção.
 */

void(timer_int_handler)() {
  counter++;
}

/**
 * @brief Lê a configuração atual de um dos timers.
 *
 * @param timer Timer cujo estado será lido (0, 1 ou 2).
 * @param st Ponteiro para armazenar o valor da configuração do timer.
 * @return 0 em caso de sucesso, 1 em caso de erro.
 */

int(timer_get_conf)(uint8_t timer, uint8_t *st) {
  if(st == NULL || timer > 2 || timer < 0) return 1;
  uint8_t cmd = (TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer));
  if (sys_outb(TIMER_CTRL, cmd) != 0) {
    return 1;
  }
  if(util_sys_inb(TIMER_0 + timer, st)) return 1;
  return 0;
}

/**
 * @brief Exibe a configuração do timer no terminal.
 *
 * @param timer Timer cujo estado será exibido (0, 1 ou 2).
 * @param conf Valor da configuração do timer.
 * @param field Campo específico da configuração a ser exibido.
 * @return 0 em caso de sucesso, 1 em caso de erro.
 */

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

/**
 * @brief Aguarda um período de tempo em milissegundos usando o timer.
 *
 * Subscreve o timer, espera um número de interrupções correspondente ao tempo desejado,
 * e depois cancela a subscrição.
 *
 * @param time_ms Tempo a aguardar, em milissegundos.
 */

void (wait_ms)(int time_ms){
  int ipcStatus;
  message msg;
  uint8_t bitNo;
  int r;

  counter = 0;
  int ticksToWait = (time_ms + 16) / 17;
  timer_subscribe_int(&bitNo);

  while (counter < ticksToWait) {
    if ((r = driver_receive(ANY, &msg, &ipcStatus)) != 0){
        continue;
    } 

    if (is_ipc_notify(ipcStatus)) {
      if (_ENDPOINT_P(msg.m_source) == HARDWARE) {
        if (msg.m_notify.interrupts & BIT(bitNo)) {
          timer_int_handler();
        }
      }
    }
  }

  timer_unsubscribe_int();
}
