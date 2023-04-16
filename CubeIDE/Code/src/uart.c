/*
    SPOKE
    
    file: uart.c
*/

#include <string.h>
#include "stm32f10x.h"
#include "main.h"
#include "uart.h"
#include "gps.h"



char uart_buffer[UART_BUF_LEN];		//raw UART data
char *backup_buf;					//backup for raw UART data



//UART Init
void uart_dma_init(void)//todo: try to use usart3 because it uses DMA channel 3, which does not mess with spi2 tx DMA ch. 5
{
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;   //ENABLE usart clock
    
    USART1->BRR = 0x0138;                   //9600 bod; mantissa 19, frac 8
    USART1->CR1 |= USART_CR1_TE;            //enable tx
    USART1->CR1 |= USART_CR1_RE;            //enable rx
    USART1->CR1 |= USART_CR1_UE;            //uart enable
    
    USART1->CR3 |= USART_CR3_DMAR;          //enable DMA mode USART
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;       //enable dma1 clock
    
    DMA1_Channel5->CPAR = (uint32_t)(&(USART1->DR));    //transfer source 
    DMA1_Channel5->CMAR = (uint32_t)(&uart_buffer[0]);  //transfer destination
    DMA1_Channel5->CNDTR = UART_BUF_LEN;                //bytes amount to receive
    
    DMA1_Channel5->CCR |= DMA_CCR5_MINC;    //enable memory increment
    DMA1_Channel5->CCR |= DMA_CCR5_TCIE;    //enable transfer complete interrupt
    DMA1_Channel5->CCR |= DMA_CCR5_EN;      //enable channel
    
    NVIC_EnableIRQ(DMA1_Channel5_IRQn);     //enable interrupts
    DMA1->IFCR = DMA_IFCR_CGIF5;            //clear all interrupt flags for DMA channel 5

    backup_buf = get_nmea_buf();
}



//Stop UART DMA
void uart_dma_stop(void)
{
    DMA1_Channel5->CCR &= ~DMA_CCR5_EN;     //disable channel
}



//Restart UART DMA
void uart_dma_restart(void)
{
    DMA1_Channel5->CNDTR = UART_BUF_LEN;    //reload bytes amount to receive
    (void)USART1->SR;						//clear ORE bit due to UART overrun occured between DMA operations
    (void)USART1->DR;
    DMA1_Channel5->CCR |= DMA_CCR5_EN;      //enable channel
}



//Backup uart buffer and then clear it
void backup_and_clear_uart_buffer(void)
{
	memcpy(backup_buf, uart_buffer, UART_BUF_LEN);
	memset(uart_buffer, 0, UART_BUF_LEN);
}



void clear_uart_buffer(void)
{
	for (uint16_t i = 0; i < UART_BUF_LEN; i++)
	{
		uart_buffer[i] = 0;
	}
}