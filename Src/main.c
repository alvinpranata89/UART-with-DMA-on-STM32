#include "stm32f4xx.h"
#include <stdio.h>
#include "dma.h"
#include "uart_dma.h"
#include <string.h>

extern uint8_t dma_rx_cmplt;
extern uint8_t dma_tx_cmplt;

extern char uart_data_buffer[UART_DATA_BUFF_SIZE];
char msg_buff[120] ={'\0'};

int main(void)
{
	uart2_rxtx_init();
	dma1_init();
	dma1_stream5_uart_rx_config();
	dma1_stream5_uart_rx_start();
	sprintf(msg_buff,"Initialization complete..\n\r");
	dma1_stream6_uart_tx_config((uint32_t)msg_buff,strlen(msg_buff));
	dma1_stream6_uart_tx_start();

    while(!dma_tx_cmplt){}

	while(1)
	{
		if(dma_rx_cmplt)
		{
			sprintf(msg_buff, "Message received : %s \r\n",uart_data_buffer);
			dma_rx_cmplt = 0;
			dma_tx_cmplt = 0;
			dma1_stream6_uart_tx_config((uint32_t)msg_buff,strlen(msg_buff));
			dma1_stream6_uart_tx_start();
		    while(!dma_tx_cmplt){}

		}

	}
}
