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
	spi1_trx(0x00);						//zero offset
	for(uint8_t i = 0; i < AIR_PACKET_LEN; i++)
	{
		spi1_trx(air_packet_tx[i]);
	}
	cs_rf_inactive();



	while ((GPIOB->IDR) & GPIO_IDR_IDR1){}		//wait

	//start tx
	cs_rf_active();
	spi1_trx(SX126X_SET_TX);			//command
	spi1_trx(0x00);						//zero timeout
	spi1_trx(0x00);
	spi1_trx(0x00);
	cs_rf_inactive();

	return 1;	//todo: check for rf mode before tx
}



//RFM98 start packet RX
uint8_t rf_start_rx(void)
{/*
	//check current mode
	uint8_t mode_before_rx = 0;
	cs_rfm98_active();
	spi1_trx(REG_OPMODE | RFM_READ);
	mode_before_rx = spi1_trx(0);
	cs_rfm98_inactive();

	if (!((mode_before_rx & (uint8_t)(~RF_OPMODE_MASK)) & 0x06))	//Start RX only in SLEEP or STANDBY mode
	{
		cs_rfm98_active();
		spi1_trx(REG_SEQCONFIG1 | RFM_WRITE);
		spi1_trx(RF_SEQCONFIG1_SEQUENCER_START | RF_SEQCONFIG1_IDLEMODE_SLEEP | RF_SEQCONFIG1_FROMSTART_TORX | RF_SEQCONFIG1_LPS_SEQUENCER_OFF);
		cs_rfm98_inactive();

		return 1;				//successful RX start
	}
	else
	{
		return 0;
	}

*/
	return 1;
}



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



//RFM98 get interrupt status
uint8_t rf_get_irq_status(void)
{/*
	//check both flag bytes
	cs_rfm98_active();
	spi1_trx(REG_IRQFLAGS1 | RFM_READ);
	spi1_trx(0);
	radio_irq_status = spi1_trx(0);		//take only RegIrqFlags2
	cs_rfm98_inactive();

	return radio_irq_status;
*/
	return 0;
}



//RFM98 get received packet
void rf_get_rx_packet(void)
{/*
	cs_rfm98_active();
	spi1_trx(REG_FIFO | RFM_READ);
	for (uint8_t i = 0; i < AIR_PACKET_LEN; i++)
	{
		air_packet_rx[i] = spi1_trx(0);
	}
	cs_rfm98_inactive();
*/}



void rf_flush_fifo(void)
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
