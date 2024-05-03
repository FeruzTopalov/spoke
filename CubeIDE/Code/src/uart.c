/*
    SPOKE
    
    file: uart.c
*/

#include <string.h>
#include "stm32f10x.h"
#include "main.h"
#include "uart.h"
#include "gps.h"



char uart_buffer[UART_BUF_LEN];		//raw UART data for GPS
char console_buffer[5] = {1, 2, 3, 4, 5};			//for console
char *backup_buf;					//backup for raw UART data



//init all uart
void uart_init(void)
{
	uart1_dma_init();
	uart3_dma_init();
}



//Console UART init
void uart1_dma_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;   //ENABLE usart clock

    USART1->BRR = 0x001A; 					//115200 bod
    USART1->CR1 |= USART_CR1_RXNEIE;    	//enable rx interrupt
    USART1->CR1 |= USART_CR1_TE;        	//enable tx
    USART1->CR1 |= USART_CR1_RE;        	//enable rx
    USART1->CR1 |= USART_CR1_UE;        	//uart enable

    USART1->CR3 |= USART_CR3_DMAT;          //enable DMA mode USART TX
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;       //enable dma1 clock

    DMA1_Channel4->CPAR = (uint32_t)(&(USART1->DR));    	//transfer destination
    DMA1_Channel4->CMAR = (uint32_t)(&console_buffer[0]);  	//transfer source
    DMA1_Channel4->CNDTR = 5;                				//bytes amount to transmit

    DMA1_Channel4->CCR |= DMA_CCR4_DIR;		//direction mem -> periph
    DMA1_Channel4->CCR |= DMA_CCR4_MINC;    //enable memory increment
    DMA1_Channel4->CCR |= DMA_CCR4_TCIE;    //enable transfer complete interrupt
    DMA1_Channel4->CCR |= DMA_CCR4_EN;      //enable channel

    NVIC_EnableIRQ(DMA1_Channel4_IRQn);     //enable DMA interrupt
    DMA1->IFCR = DMA_IFCR_CGIF4;            //clear all interrupt flags for DMA channel 3

    NVIC_EnableIRQ(USART1_IRQn);			//enable UART RX interrupt
}



void uart1_dma_start(void)
{
	DMA1_Channel4->CNDTR = 5;
	DMA1_Channel4->CCR |= DMA_CCR4_EN;      //enable channel
}



void uart1_dma_stop(void)
{
	DMA1_Channel4->CCR &= ~DMA_CCR4_EN;     //disable channel
}



void uart1_tx_byte(uint8_t tx_data)		//todo: implement through dma1 channel 4 USART1_TX
{
    while(!(USART1->SR & USART_SR_TXE))     //wait for transmit register empty
    {
    }
    USART1->DR = tx_data;                      //transmit
}



//GPS UART Init
void uart3_dma_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;   //ENABLE usart clock
    
    //see gpio.c for this pin
    //PB10 - USART3 TX (GPS)
    GPIOB->CRH &= ~GPIO_CRH_MODE10_0;    //output 2 MHz
    GPIOB->CRH |= GPIO_CRH_MODE10_1;
    GPIOB->CRH &= ~GPIO_CRH_CNF10_0;     //alternate output push-pull
    GPIOB->CRH |= GPIO_CRH_CNF10_1;

    USART3->BRR = 0x0138;                   //9600 bod; mantissa 19, frac 8
    USART3->CR1 |= USART_CR1_TE;            //enable tx
    USART3->CR1 |= USART_CR1_RE;            //enable rx
    USART3->CR1 |= USART_CR1_UE;            //uart enable
    
    USART3->CR3 |= USART_CR3_DMAR;          //enable DMA mode USART RX
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;       //enable dma1 clock
    
    DMA1_Channel3->CPAR = (uint32_t)(&(USART3->DR));    //transfer source
    DMA1_Channel3->CMAR = (uint32_t)(&uart_buffer[0]);  //transfer destination
    DMA1_Channel3->CNDTR = UART_BUF_LEN;                //bytes amount to receive
    
    DMA1_Channel3->CCR |= DMA_CCR3_MINC;    //enable memory increment
    DMA1_Channel3->CCR |= DMA_CCR3_TCIE;    //enable transfer complete interrupt
    DMA1_Channel3->CCR |= DMA_CCR3_EN;      //enable channel
    
    NVIC_EnableIRQ(DMA1_Channel3_IRQn);     //enable interrupts
    DMA1->IFCR = DMA_IFCR_CGIF3;            //clear all interrupt flags for DMA channel 3

    backup_buf = get_nmea_buf();
}



//Stop UART DMA
void uart3_dma_stop(void)
{
    DMA1_Channel3->CCR &= ~DMA_CCR3_EN;     //disable channel
}



//Restart UART DMA
void uart3_dma_restart(void)
{
    DMA1_Channel3->CNDTR = UART_BUF_LEN;    //reload bytes amount to receive
    (void)USART3->SR;						//clear ORE bit due to UART overrun occured between DMA operations
    (void)USART3->DR;
    DMA1_Channel3->CCR |= DMA_CCR3_EN;      //enable channel
}



//Backup uart buffer and then clear it
void backup_and_clear_uart_buffer(void)
{
	memcpy(backup_buf, uart_buffer, UART_BUF_LEN);
	memset(uart_buffer, 0, UART_BUF_LEN);
}



void clear_uart_buffer(void)
{
	memset(uart_buffer, 0, UART_BUF_LEN);
}



void uart3_tx_byte(uint8_t tx_data)
{
    while(!(USART3->SR & USART_SR_TXE))     //wait for transmit register empty
    {
    }
    USART3->DR = tx_data;                      //transmit
}
