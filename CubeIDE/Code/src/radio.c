/*
	SPOKE
    
    file: radio.c
*/

#include "stm32f10x.h"
#include "radio.h"
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
#define AIR_PACKET_LEN      (12)



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



//sx1268 Init
void rf_init(void)
{
	print_debug("RF init start\n");

    cs_rf_inactive();       	//set pins initial state
    res_rf_inactive();
    rf_rx_mode();
    delay_cyc(100000);
    
    res_rf_active();        	//reset the chip
    delay_cyc(100000);
    res_rf_inactive();
    delay_cyc(100000);



    while ((GPIOB->IDR) & GPIO_IDR_IDR1){}		//wait BUSY goes low	todo: move to spi1_trx()

    //clear errors
    cs_rf_active();
    spi1_trx(SX126X_CLR_DEVICE_ERRORS);      	//send command byte
    spi1_trx(SX126X_NOP);			//send nop
    spi1_trx(SX126X_NOP);			//send nop
    cs_rf_inactive();



    while ((GPIOB->IDR) & GPIO_IDR_IDR1){}		//wait BUSY goes low	todo: move to spi1_trx()

    //get status
    uint8_t status;
    cs_rf_active();
    spi1_trx(SX126X_GET_STATUS);      	//send command byte
    status = spi1_trx(SX126X_NOP);			//send nop, get response
    cs_rf_inactive();

    //debug status
    char buf[10] = {0};
    print_debug("RF status power-up: ");
    itoa32(status, buf);
    print_debug(buf);
    uart1_tx_byte('\n');



    while ((GPIOB->IDR) & GPIO_IDR_IDR1){}		//wait BUSY goes low	todo: move to spi1_trx()

    //set dio3 as dc source for tcxo
    cs_rf_active();
    spi1_trx(SX126X_SET_DIO3_AS_TCXO_CTRL);      	//send command byte
    spi1_trx(TCXO_CTRL_1_8V);			//send nop, get response
    spi1_trx(0x00);							//set delay of 100 ms (6400 in 15.625 us steps)
    spi1_trx(0x19);
    spi1_trx(0x00);
    cs_rf_inactive();

    delay_cyc(100000);



    while ((GPIOB->IDR) & GPIO_IDR_IDR1){}		//wait BUSY goes low	todo: move to spi1_trx()

    //set status
    cs_rf_active();
    spi1_trx(SX126X_SET_STANDBY);      	//send command byte
    spi1_trx(STDBY_XOSC);			//set 32M osc
    cs_rf_inactive();



    while ((GPIOB->IDR) & GPIO_IDR_IDR1){}		//wait BUSY goes low	todo: move to spi1_trx()

    //get status
    cs_rf_active();
    spi1_trx(SX126X_GET_STATUS);      	//send command byte
    status = spi1_trx(SX126X_NOP);			//send nop, get response
    cs_rf_inactive();

    //debug status again
    print_debug("RF status changed to: ");
    itoa32(status, buf);
    print_debug(buf);
    uart1_tx_byte('\n');







    while ((GPIOB->IDR) & GPIO_IDR_IDR1){}		//wait BUSY goes low	todo: move to spi1_trx()

    //get errors
    cs_rf_active();
    spi1_trx(SX126X_GET_DEVICE_ERRORS);      	//send command byte
    spi1_trx(SX126X_NOP);			//send nop
    spi1_trx(SX126X_NOP);			//send nop
    status = spi1_trx(SX126X_NOP);			//send nop, get errors lsb
    cs_rf_inactive();

    //debug status again
	print_debug("RF errors lsb: ");
	itoa32(status, buf);
	print_debug(buf);
	uart1_tx_byte('\n');

/*
    uint8_t init_arr[] = RFM_CONF_ARRAY;    	//array with init data
    
    for (uint8_t i = 0; i < sizeof(init_arr); i += 2)
    {
        cs_rfm98_active();
        spi1_trx(init_arr[i]);      	//send command byte
        spi1_trx(init_arr[i + 1]);		//send value
        cs_rfm98_inactive();
    }

    //Get current settings
    p_settings = get_settings();

    //Set the packet len
    cs_rfm98_active();
	spi1_trx(REG_PAYLOADLENGTH | RFM_WRITE);
	spi1_trx(AIR_PACKET_LEN);
	cs_rfm98_inactive();

	//Set frequency
	freq_reg_value_float = ((float)BASE_CHANNEL_FREQUENCY + p_settings->freq_channel * CHANNEL_FREQUENCY_STEP) / ((float)RFM98_CRYSTAL / POWER_2_TO_19);
	freq_reg_value.as_uint32 = freq_reg_value_float;
    cs_rfm98_active();
	spi1_trx(REG_FRFMSB | RFM_WRITE);
	spi1_trx(freq_reg_value.as_array[2]);
	spi1_trx(freq_reg_value.as_array[1]);
	spi1_trx(freq_reg_value.as_array[0]);
	cs_rfm98_inactive();

	//Set TX power
	switch (p_settings->tx_power_opt)
	{
		case TX_POWER_1MILLIW_SETTING:
			pa_conf_reg_value = (RF_PACONFIG_PASELECT_PABOOST | RF_PACONFIG_MAX_POWER_7 | RF_PACONFIG_OUTPUTPOWER_0);
			pa_dac_reg_value = RF_PADAC_20DBM_OFF;
			break;

		case TX_POWER_10MILLIW_SETTING:
			pa_conf_reg_value = (RF_PACONFIG_PASELECT_PABOOST | RF_PACONFIG_MAX_POWER_7 | RF_PACONFIG_OUTPUTPOWER_8);
			pa_dac_reg_value = RF_PADAC_20DBM_OFF;
			break;

		case TX_POWER_50MILLIW_SETTING:
			pa_conf_reg_value = (RF_PACONFIG_PASELECT_PABOOST | RF_PACONFIG_MAX_POWER_7 | RF_PACONFIG_OUTPUTPOWER_15);
			pa_dac_reg_value = RF_PADAC_20DBM_OFF;
			break;

		case TX_POWER_100MILLIW_SETTING:
			pa_conf_reg_value = (RF_PACONFIG_PASELECT_PABOOST | RF_PACONFIG_MAX_POWER_7 | RF_PACONFIG_OUTPUTPOWER_15);
			pa_dac_reg_value = RF_PADAC_20DBM_ON;
			break;

		default:	//10 mW
			pa_conf_reg_value = (RF_PACONFIG_PASELECT_PABOOST | RF_PACONFIG_MAX_POWER_7 | RF_PACONFIG_OUTPUTPOWER_8);
			pa_dac_reg_value = RF_PADAC_20DBM_OFF;
			break;
	}

    cs_rfm98_active();
	spi1_trx(REG_PACONFIG | RFM_WRITE);
	spi1_trx(pa_conf_reg_value);
	cs_rfm98_inactive();

	cs_rfm98_active();
	spi1_trx(REG_PADAC | RFM_WRITE);
	spi1_trx(pa_dac_reg_value);
	cs_rfm98_inactive();

	//Calibrate IQ
	cs_rfm98_active();
	spi1_trx(REG_OPMODE | RFM_WRITE);
	spi1_trx(RF_OPMODE_LONGRANGEMODE_OFF | RF_OPMODE_MODULATIONTYPE_FSK | RF_OPMODE_STANDBY);
	cs_rfm98_inactive();

	cs_rfm98_active();
	spi1_trx(REG_IMAGECAL | RFM_WRITE);
	spi1_trx(RF_IMAGECAL_AUTOIMAGECAL_OFF | RF_IMAGECAL_IMAGECAL_START | RF_IMAGECAL_TEMPMONITOR_OFF);
	cs_rfm98_inactive();

	uint8_t cal_iq_running;
	do
	{
		delay_cyc(100);
		cs_rfm98_active();
		spi1_trx(REG_IMAGECAL | RFM_READ);
		cal_iq_running = spi1_trx(0) & RF_IMAGECAL_IMAGECAL_RUNNING;
		cs_rfm98_inactive();
	}
	while (cal_iq_running == RF_IMAGECAL_IMAGECAL_RUNNING);

	cs_rfm98_active();
	spi1_trx(REG_OPMODE | RFM_WRITE);
	spi1_trx(RF_OPMODE_LONGRANGEMODE_OFF | RF_OPMODE_MODULATIONTYPE_FSK | RF_OPMODE_SLEEP);
	cs_rfm98_inactive();
*/}



//RFM98 TX packet
uint8_t rf_tx_packet(void)
{/*
	//check current mode
	uint8_t mode_before_tx = 0;
	cs_rfm98_active();
	spi1_trx(REG_OPMODE | RFM_READ);
	mode_before_tx = spi1_trx(0);
	cs_rfm98_inactive();

	if (!((mode_before_tx & (uint8_t)(~RF_OPMODE_MASK)) & 0x06))	//Start TX only in SLEEP or STANDBY mode
	{
		//fill TX FIFO buffer
		cs_rfm98_active();
		spi1_trx(REG_FIFO | RFM_WRITE);
		for(uint8_t i = 0; i < AIR_PACKET_LEN; i++)
		{
			spi1_trx(air_packet_tx[i]);
		}
		cs_rfm98_inactive();

		//start TX
		cs_rfm98_active();
		spi1_trx(REG_SEQCONFIG1 | RFM_WRITE);
		spi1_trx(RF_SEQCONFIG1_SEQUENCER_START | RF_SEQCONFIG1_IDLEMODE_SLEEP | RF_SEQCONFIG1_FROMSTART_TOTX | RF_SEQCONFIG1_LPS_SEQUENCER_OFF | RF_SEQCONFIG1_FROMTX_TOLPS);
		cs_rfm98_inactive();

		return 1;				//successful TX start
	}
	else
	{
		return 0;
	}
*/
	return 1;
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
