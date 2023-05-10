/*
	SPOKE
    
    file: radio.c
*/

#include "stm32f10x.h"
#include "radio.h"
#include "sx126x.h"
#include "sx126x_config.h"
#include "gpio.h"
#include "spi.h"
#include "service.h"
#include "main.h"
#include "settings.h"
#include "uart.h"



#define BASE_CHANNEL_FREQUENCY 			(433050000)
#define CHANNEL_FREQUENCY_STEP			(25000)
#define RFM98_CRYSTAL					(32000000)
#define POWER_2_TO_19					(524288)




//bytes amount to tx/rx over air; does not include preamble, syncword and CRC
//1 byte device number and ID (single char)
//1 byte flags
//4 bytes lat
//4 bytes lon
//2 bytes altitude
#define AIR_PACKET_LEN      (FSK_PP7_PLOAD_LEN_12_BYTE)



union
{
    uint32_t as_uint32;
    uint8_t as_array[4];
} freq_reg_value;
float freq_reg_value_float;

uint8_t pa_conf_reg_value;
uint8_t pa_dac_reg_value;

uint8_t radio_irq_status = 0;
uint8_t air_packet_tx[AIR_PACKET_LEN];
uint8_t air_packet_rx[AIR_PACKET_LEN];
struct settings_struct *p_settings;



//sx126x Init
void rf_init(void)
{
    cs_rf_inactive();       	//set pins initial state
    res_rf_inactive();
    rf_rx_mode();
    
    res_rf_active();        	//reset the chip
    delay_cyc(100000);
    res_rf_inactive();
    while ((GPIOB->IDR) & GPIO_IDR_IDR1){}		//wait reset done		todo: move to cs_rf_active()



    uint8_t rf_init_arr[] = SX126X_CONFIG_ARRAY;    	//array with init data
    uint8_t i = 0;
    uint8_t len = 0;
    
    while (rf_init_arr[i] != 0x00)
    {
        len = rf_init_arr[i++];

        while ((GPIOB->IDR) & GPIO_IDR_IDR1){}		//wait

        cs_rf_active();
        while (len--)
        {
            spi1_trx(rf_init_arr[i++]);
        }
        cs_rf_inactive();
    }

}



//sx126x TX packet
uint8_t rf_tx_packet(void)
{

	while ((GPIOB->IDR) & GPIO_IDR_IDR1){}		//wait

	//fill tx buffer
	cs_rf_active();
	spi1_trx(SX126X_WRITE_BUFFER);		//command
	spi1_trx(BASE_ADDR_TX);				//offset
	for(uint8_t i = 0; i < AIR_PACKET_LEN; i++)
	{
		spi1_trx(air_packet_tx[i]);
	}
	cs_rf_inactive();



	while ((GPIOB->IDR) & GPIO_IDR_IDR1){}		//wait

	//enable TX path
	rf_tx_mode();

	//start tx
	cs_rf_active();
	spi1_trx(SX126X_SET_TX);			//command
	spi1_trx(TX_TIMEOUT_DISABLED_2);	//zero timeout
	spi1_trx(TX_TIMEOUT_DISABLED_1);
	spi1_trx(TX_TIMEOUT_DISABLED_0);
	cs_rf_inactive();

	return 1;	//todo: check for rf mode before tx
}



//sx126x start packet RX
uint8_t rf_start_rx(void)
{
	while ((GPIOB->IDR) & GPIO_IDR_IDR1){}		//wait

	//enable RX path
	rf_rx_mode();

	//start rx
	cs_rf_active();
	spi1_trx(SX126X_SET_RX);			//command
	spi1_trx(RX_TIMEOUT_45MS_2);		//45 ms timeout
	spi1_trx(RX_TIMEOUT_45MS_1);
	spi1_trx(RX_TIMEOUT_45MS_0);
	cs_rf_inactive();

	return 1;
}



//sx126x get received packet
void rf_get_rx_packet(void)
{
	while ((GPIOB->IDR) & GPIO_IDR_IDR1){}		//wait

	//get data
	cs_rf_active();
	spi1_trx(SX126X_READ_BUFFER);	//command
	spi1_trx(BASE_ADDR_RX);			//offset
	spi1_trx(SX126X_NOP);			//NOP required
	for (uint8_t i = 0; i < AIR_PACKET_LEN; i++)
	{
		air_packet_rx[i] = spi1_trx(SX126X_NOP);
	}
	cs_rf_inactive();
}



//sx126x get status
uint8_t rf_get_status(void)
{
    while ((GPIOB->IDR) & GPIO_IDR_IDR1){}

    uint8_t status;
    cs_rf_active();
    spi1_trx(SX126X_GET_STATUS);		//send command byte
    status = spi1_trx(SX126X_NOP);		//send nop, get response
    cs_rf_inactive();

    return status;
}



//sx126x get interrupt status
uint8_t rf_get_irq_status(void)
{
    while ((GPIOB->IDR) & GPIO_IDR_IDR1){}

    uint8_t status;
    cs_rf_active();
    spi1_trx(SX126X_GET_IRQ_STATUS);	//send command byte
    spi1_trx(SX126X_NOP);
    spi1_trx(SX126X_NOP);				//MSB is not used yet
    status = spi1_trx(SX126X_NOP);		//get only LSB of IRQ status
    cs_rf_inactive();

    return status;
}



//sx126x clear interrupts
void rf_clear_irq(void)
{
    while ((GPIOB->IDR) & GPIO_IDR_IDR1){}

    cs_rf_active();
    spi1_trx(SX126X_CLR_IRQ_STATUS);	//send command byte
    spi1_trx(IRQ_MASK_ALL);				//clear all interrupts
    spi1_trx(IRQ_MASK_ALL);
    cs_rf_inactive();
}








void rf_flush_fifo(void)	//todo: delete
{/*
	cs_rfm98_active();
	spi1_trx(REG_IRQFLAGS2 | RFM_WRITE);
	spi1_trx(RF_IRQFLAGS2_FIFOOVERRUN);		//Clear FIFO by writing overrun flag
	cs_rfm98_inactive();
*/}



uint8_t *get_air_packet_tx(void)
{
	return &air_packet_tx[0];
}



uint8_t *get_air_packet_rx(void)
{
	return &air_packet_rx[0];
}
