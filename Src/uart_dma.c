#include "uart_dma.h"

#define UART2EN			(1U<<17)
#define GPIOAEN			(1U<<0)

#define CR1_TE			(1U<<3)
#define CR1_RE			(1U<<2)
#define CR1_UE			(1U<<13)
#define SR_TXE			(1U<<7)

#define CR3_DMAT		(1U<<7)
#define CR3_DMAR		(1U<<6)
#define SR_TC			(1U<<6)
#define CR1_TCIE		(1U<<6)

#define UART_BAUDRATE	115200
#define CLK				16000000

#define DMA1EN			    (1U<<21)
#define DMA_SCR_EN  		(1U<<0)
#define DMA_SCR_MINC		(1U<<10)
#define DMA_SCR_PINC		(1U<<9)
#define DMA_SCR_CIRC		(1U<<8)
#define DMA_SCR_TCIE		(1U<<4)
#define DMA_SCR_TEIE		(1U<<2)
#define DMA_SFCR_DMDIS		(1U<<2)

#define HIFCR_CDMEIF5		(1U<<8)
#define HIFCR_CTEIF5		(1U<<9)
#define HIFCR_CTCIF5		(1U<<11)

#define HIFCR_CDMEIF6		(1U<<18)
#define HIFCR_CTEIF6		(1U<<19)
#define HIFCR_CTCIF6		(1U<<21)

#define HIFSR_TCIF5		(1U<<11)
#define HIFSR_TCIF6		(1U<<21)

static uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate);
static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate);

char uart_data_buffer[UART_DATA_BUFF_SIZE];

uint8_t dma_rx_cmplt;
uint8_t dma_tx_cmplt;

//uint8_t g_uart_cmplt;

void uart2_rxtx_init(void)
{
	//--------GPIO PIN Config----------------//
	//Enable clock access to GPIOA
		RCC->AHB1ENR |= GPIOAEN;

	//Set PA2 mode to alternate function mode
	    GPIOA->MODER &= ~(1U<<4);
		GPIOA->MODER |=	 (1U<<5);

	//Set PA3 to AF mode
		GPIOA->MODER &= ~(1U<<6);
		GPIOA->MODER |=	 (1U<<7);

	//Set PA2 AF mode to AF7
		GPIOA->AFR[0] |= (1U<<8);
		GPIOA->AFR[0] |= (1U<<9);
		GPIOA->AFR[0] |= (1U<<10);
		GPIOA->AFR[0] &= ~(1U<<11);

	//Set PA3 AF mode to AF7
		GPIOA->AFR[0] |= (1U<<12);
		GPIOA->AFR[0] |= (1U<<13);
		GPIOA->AFR[0] |= (1U<<14);
		GPIOA->AFR[0] &= ~(1U<<15);

	//--------UART Module Config----------------//
	//Enable clock access to UART2
		RCC->APB1ENR |= UART2EN;

	//set baudrate
		uart_set_baudrate(CLK,UART_BAUDRATE);

	//enable UART to use DMA on TX and RX
		USART2->CR3 = CR3_DMAT | CR3_DMAR;

	//set transfer direction
		USART2->CR1 = CR1_TE | CR1_RE;

	//clear Transfer Complete Flag
		USART2->SR &=~SR_TC;

	//Enable Transfer Complete Interrupt
		USART2->CR1 |= CR1_TCIE;

	//Enable UART module
		USART2->CR1 |= CR1_UE;
}


void dma1_init(void)
{
	//enable clock access to DMA1
		RCC->AHB1ENR |= DMA1EN;
	//enable DMA stream6 interrupt in NVIC
		NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

void dma1_stream5_uart_rx_config(void)
{
	//Disable DMA stream
		DMA1_Stream5->CR &=~DMA_SCR_EN;
	//wait until DMA is disabled
		while((DMA1_Stream5->CR & DMA_SCR_EN)){}

	//clear interrupt flag for stream 5
		DMA1->HIFCR = HIFCR_CDMEIF5 |HIFCR_CTEIF5|HIFCR_CTCIF5;

	//set periph address
		DMA1_Stream5->PAR = (uint32_t)(&USART2->DR);

	//set mem address
		DMA1_Stream5->M0AR = (uint32_t)(uart_data_buffer);

	//set number of transfer
		DMA1_Stream5->NDTR = (uint16_t)UART_DATA_BUFF_SIZE;

	//select channel 4
		DMA1_Stream5->CR &=~(1U<<25);
		DMA1_Stream5->CR &=~(1U<<26);
		DMA1_Stream5->CR |= (1U<<27);

	//enable memory address increment
		DMA1_Stream5->CR |=DMA_SCR_MINC;

	//enable circular mode
		DMA1_Stream5->CR |=DMA_SCR_CIRC;

	//enable transfer complete interrupt
		DMA1_Stream5->CR |= DMA_SCR_TCIE;

	//set transfer direction (periph to mem)
		DMA1_Stream5->CR &=~(1U<<6);
		DMA1_Stream5->CR &=~(1U<<7);



	//enable DMA stream5 interrupt in NVIC
		NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

void dma1_stream6_uart_tx_config(uint32_t msg_to_snd, uint32_t msg_len)
{
		//Disable DMA stream
			DMA1_Stream6->CR &=~DMA_SCR_EN;

		//wait until DMA is disabled
			while((DMA1_Stream6->CR & DMA_SCR_EN)){}

		//clear interrupt flag for stream 6
			DMA1->HIFCR = HIFCR_CDMEIF6 |HIFCR_CTEIF6|HIFCR_CTCIF6;

		//set periph address
			DMA1_Stream6->PAR = (uint32_t)(&(USART2->DR));

		//set mem address
			DMA1_Stream6->M0AR = msg_to_snd;

		//set number of transfer
			DMA1_Stream6->NDTR = msg_len;

		//select channel 4
			DMA1_Stream6->CR &= ~(1u<<25);
			DMA1_Stream6->CR &= ~(1u<<26);
			DMA1_Stream6->CR |= (1u<<27);

		//enable memory address increment
			DMA1_Stream6->CR |=DMA_SCR_MINC;

		//set transfer direction
			DMA1_Stream6->CR |=(1U<<6);
			DMA1_Stream6->CR &=~(1U<<7);

		//enable transfer complete interrupt
			DMA1_Stream6->CR |= DMA_SCR_TCIE;


}

void dma1_stream5_uart_rx_start (void)
{
	//enable dma stream
	DMA1_Stream5->CR |= DMA_SCR_EN;
}

void dma1_stream6_uart_tx_start (void)
{
	//enable dma stream
	DMA1_Stream6->CR |= DMA_SCR_EN;
}

static uint16_t compute_uart_bd(uint32_t periph_clk, uint32_t baudrate)
{
	return ((periph_clk +( baudrate/2U ))/baudrate);
}


static void uart_set_baudrate(uint32_t periph_clk, uint32_t baudrate)
{
	USART2->BRR  = compute_uart_bd(periph_clk,baudrate);
}

//If TX is complete, this interrupt handler will be executed
void DMA1_Stream6_IRQHandler(void)
{
	if((DMA1->HISR) & HIFSR_TCIF6)
	{
		//do_something
		dma_tx_cmplt = 1;

		/*Clear the flag*/
		DMA1->HIFCR |= HIFCR_CTCIF6;
	}
}

//If RX is complete, this interrupt handler will be executed
void DMA1_Stream5_IRQHandler(void)
{
	if((DMA1->HISR) & HIFSR_TCIF5)
	{
		//do_something
		dma_rx_cmplt = 1;

		/*Clear the flag*/
		DMA1->HIFCR |= HIFCR_CTCIF5;
	}
}
