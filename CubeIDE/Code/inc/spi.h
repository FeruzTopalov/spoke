/*
    SPOKE
    
    file: spi.h
*/



#ifndef SPI_HEADER
#define SPI_HEADER



void spi_init(void);

void spi1_init(void);
void spi1_clock_disable(void);
void spi1_clock_enable(void);
uint8_t spi1_trx(uint8_t send_data);

void spi2_init(void);
void spi2_clock_disable(void);
void spi2_clock_enable(void);
void spi2_trx(uint8_t send_data);



#endif /*SPI_HEADER*/
