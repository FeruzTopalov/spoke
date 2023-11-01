/*
    SPOKE
    
    file: uart.h
*/



#ifndef UART_HEADER
#define UART_HEADER



#define UART_BUF_LEN        (1024)



void uart1_init(void);
void uart1_tx_byte(uint8_t tx_data);

void uart3_dma_init(void);
void uart3_dma_stop(void);
void uart3_dma_restart(void);
void backup_and_clear_uart_buffer(void);
void clear_uart_buffer(void);



#endif /*UART_HEADER*/
