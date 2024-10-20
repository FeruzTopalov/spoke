/*
    SPOKE
    
    file: spi.c
*/

#include "stm32f10x.h"
#include "spi.h"
#include "bit_band.h"
#include "lcd.h"



void spi_init(void)
{
	spi1_init();
	spi2_dma_init(get_lcd_buffer(), LCD_SIZE_X);
}



//Init SPI1
void spi1_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;             //enable clock spi1
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;             //enable alternate function clock
    SPI1->CR1 &= ~SPI_CR1_BR;                       //clock/2
    SPI1->CR1 |= SPI_CR1_SSM;
    SPI1->CR1 |= SPI_CR1_SSI;
    SPI1->CR1 |= SPI_CR1_MSTR;                      //master mode
    SPI1->CR1 &= ~SPI_CR1_CPOL;                     //SCK = 0 in IDLE
    SPI1->CR1 &= ~SPI_CR1_CPHA;                     //first rising edge capture
    SPI1->CR1 |= SPI_CR1_SPE;                       //enable spi

    spi1_clock_enable();
}



void spi1_clock_disable(void)
{
	BIT_BAND_PERI(RCC->APB2ENR, RCC_APB2ENR_SPI1EN) = 0;
}



void spi1_clock_enable(void)
{
	BIT_BAND_PERI(RCC->APB2ENR, RCC_APB2ENR_SPI1EN) = 1;
}



//TRX one byte via SPI1
uint8_t spi1_trx(uint8_t send_data)
{
    while(!(SPI1->SR & SPI_SR_TXE)){}       //while TXE bit = 0
    SPI1->DR = send_data;

    while(!(SPI1->SR & SPI_SR_RXNE)){}      //while RXNE = 0

    while(SPI1->SR & SPI_SR_BSY){}      //while busy

    return SPI1->DR;
}



//Init SPI2
void spi2_dma_init(uint8_t buffer[], uint32_t buffer_len)
{
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;     //enable clock spi2
    SPI2->CR1 &= ~SPI_CR1_BR;               //clock/2
    SPI2->CR1 |= SPI_CR1_SSM;				//disable software slave management
    SPI2->CR1 |= SPI_CR1_SSI;
    SPI2->CR1 |= SPI_CR1_MSTR;              //master mode (tx only, MISO is used as a general i/o)
    SPI2->CR1 &= ~SPI_CR1_CPOL;             //SCK = 0 in IDLE
    SPI2->CR1 &= ~SPI_CR1_CPHA;             //first rising edge capture
    SPI2->CR1 |= SPI_CR1_SPE;               //enable spi

    SPI2->CR2 |= SPI_CR2_TXDMAEN;			//enable SPI TX DMA
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;       //enable dma1 clock

    DMA1_Channel5->CMAR = (uint32_t)(&buffer[0]);    //transfer source
    DMA1_Channel5->CPAR = (uint32_t)(&(SPI2->DR));  //transfer destination
    DMA1_Channel5->CNDTR = buffer_len;                //bytes amount to transmit

    DMA1_Channel5->CCR |= DMA_CCR5_DIR;		//from memory to periph
    DMA1_Channel5->CCR |= DMA_CCR5_MINC;    //enable memory increment
    DMA1_Channel5->CCR |= DMA_CCR5_TCIE;    //enable transfer complete interrupt

    NVIC_EnableIRQ(DMA1_Channel5_IRQn);     //enable interrupts
    DMA1->IFCR = DMA_IFCR_CGIF5;            //clear all interrupt flags for DMA channel 5

    //spi2_clock_disable();
}



void spi2_dma_start(uint8_t buffer[], uint32_t buffer_len)
{
	DMA1_Channel5->CMAR = (uint32_t)(&buffer[0]);    //transfer source
	DMA1_Channel5->CNDTR = buffer_len;                //bytes amount to transmit
    DMA1_Channel5->CCR |= DMA_CCR5_EN;      //enable channel
}



void spi2_dma_stop(void)
{
	DMA1_Channel5->CCR &= ~DMA_CCR5_EN;      //disable channel
}



void spi2_clock_disable(void)
{
	BIT_BAND_PERI(RCC->APB1ENR, RCC_APB1ENR_SPI2EN) = 0;
}



void spi2_clock_enable(void)
{
	BIT_BAND_PERI(RCC->APB1ENR, RCC_APB1ENR_SPI2EN) = 1;
}



//TX one byte via SPI2, no reception from LCD needed
void spi2_tx(uint8_t send_data)
{
    while (!(SPI2->SR & SPI_SR_TXE)){}      //while TXE bit = 0
    SPI2->DR = send_data;

    while(SPI2->SR & SPI_SR_BSY){}      //while busy
}
