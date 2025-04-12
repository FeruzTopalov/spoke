/*
    SPOKE
    
    file: uart.h
*/



#ifndef UART_HEADER
#define UART_HEADER



#define MAX_UART_BUF_LEN        (5800)



void uart_init(void);

void uart1_dma_init(void);
void uart1_dma_start(void);
void uart1_dma_stop(void);
void uart1_tx_byte(uint8_t tx_data);
void toggle_console_reports(uint8_t enabled);
void report_to_console(void);

void uart3_dma_init(void);
void uart3_dma_stop(void);
void uart3_dma_restart(void);
void uart3_tx_byte(uint8_t tx_data);
void backup_and_clear_uart_buffer(void);
void clear_uart_buffer(void);
uint16_t get_gps_uart_buf_len(void);



#endif /*UART_HEADER*/
