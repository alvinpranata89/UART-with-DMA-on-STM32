#ifndef __UART_DMA_H
#define __UART_DMA_H

#include "stm32f4xx.h"
#define UART_DATA_BUFF_SIZE	5

void dma1_stream6_uart_tx_config(uint32_t msg_to_snd, uint32_t msg_len);
void dma1_stream6_uart_tx_start (void);
void uart2_rxtx_init(void);
void dma1_stream5_uart_rx_config(void);
void dma1_stream5_uart_rx_start (void);
void dma1_init(void);

#endif
