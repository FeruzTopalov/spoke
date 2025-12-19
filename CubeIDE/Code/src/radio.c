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
#include "config.h"



void rf_wait_busy(void);
void rf_workaround_15_1(void);
void rf_workaround_15_2(void);
void rf_workaround_15_4(void);
void rf_tx_power_test(void);
void rf_config_frequency(uint8_t channel_num);
void rf_config_tx_power(int8_t power_dbm);
void rf_set_cw_tx(void);

//successfull tx/rx!

#define RADIO_CRYSTAL					(32000000)
#define POWER_2_TO_25					(33554432)



//Bytes amount to tx/rx over the air; does not include preamble, syncword and CRC
//1 byte device number and ID (single char)
//1 byte flags
//4 bytes lat
//4 bytes lon
//2 bytes altitude
//TOTAL 12 bytes
#define AIR_PACKET_LEN      (LORA_PP4_PLOAD_LEN_13_BYTE)



union
{
    uint32_t as_uint32;
    uint8_t as_array[4];
} freq_reg;

uint8_t air_packet_tx[AIR_PACKET_LEN];
uint8_t air_packet_rx[AIR_PACKET_LEN];
struct settings_struct *p_settings;



//wait until radio is ready to accept a command, or until reset is done
void rf_wait_busy(void)
{
	while ((GPIOB->IDR) & GPIO_IDR_IDR1){}
}



//sx126x Init
void rf_init(void)
{
    cs_rf_inactive();       	//set pins initial state
    res_rf_inactive();
    rf_rx_mode();
    
    res_rf_active();        	//reset the chip
    delay_cyc(20);
    res_rf_inactive();
    rf_wait_busy();

    uint8_t rf_init_arr[] = SX126X_CONFIG_ARRAY;    	//array with init data
    uint8_t i = 0;
    uint8_t len = 0;
    
    spi1_clock_enable();
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
    spi1_clock_disable();

    //required workarounds
    rf_workaround_15_2();
    rf_workaround_15_4();

    //Get current settings
    p_settings = get_settings();

    //Set freq
    rf_config_frequency(p_settings->freq_channel);

	//Set TX power
    int8_t power_reg_dbm;
	switch (p_settings->tx_power_opt)
	{
		case TX_POWER_NEG9DBM_SETTING:
			power_reg_dbm = -9;
			break;

		case TX_POWER_POS0DBM_SETTING:
			power_reg_dbm = 0;
			break;

		case TX_POWER_POS10DBM_SETTING:
			power_reg_dbm = 10;
			break;

		case TX_POWER_POS14DBM_SETTING:
			power_reg_dbm = 14;
			break;

		case TX_POWER_POS22DBM_SETTING:
			power_reg_dbm = 20;
			break;

		default:	//10 mW
			power_reg_dbm = 10;
			break;
	}
	rf_config_tx_power(power_reg_dbm);
}



void rf_config_frequency(uint8_t channel_num)
{
    float freq_reg_value;
	freq_reg_value = ((float)BASE_CHANNEL_FREQUENCY + channel_num * CHANNEL_FREQUENCY_STEP) / ((float)RADIO_CRYSTAL / POWER_2_TO_25);
	freq_reg.as_uint32 = freq_reg_value;

	spi1_clock_enable();

	cs_rf_active();
	spi1_trx(SX126X_SET_RF_FREQUENCY);			//command
	spi1_trx(freq_reg.as_array[3]);
	spi1_trx(freq_reg.as_array[2]);
	spi1_trx(freq_reg.as_array[1]);
	spi1_trx(freq_reg.as_array[0]);
	cs_rf_inactive();

	spi1_clock_disable();
}



void rf_config_tx_power(int8_t power_dbm)
{
	spi1_clock_enable();

	cs_rf_active();
	spi1_trx(SX126X_SET_TX_PARAMS);			//command
	spi1_trx(power_dbm);
	spi1_trx(TX_RAMP_TIME_800U);
	cs_rf_inactive();

	spi1_clock_disable();
}



//sx126x TX packet
uint8_t rf_tx_packet(void)
{
	rf_workaround_15_1();

	spi1_clock_enable();

	//fill tx buffer
	cs_rf_active();
	spi1_trx(SX126X_WRITE_BUFFER);		//command
	spi1_trx(BASE_ADDR_TX);				//offset
	for(uint8_t i = 0; i < AIR_PACKET_LEN; i++)
	{
		spi1_trx(air_packet_tx[i]);
	}
	cs_rf_inactive();


	//enable TX path
	rf_tx_mode();

	//start tx
	cs_rf_active();
	spi1_trx(SX126X_SET_TX);			//command
	spi1_trx(TX_TIMEOUT_DISABLED_2);	//zero timeout
	spi1_trx(TX_TIMEOUT_DISABLED_1);
	spi1_trx(TX_TIMEOUT_DISABLED_0);
	cs_rf_inactive();

	spi1_clock_disable();

	return 1;
}



//sx126x start packet RX
uint8_t rf_start_rx(void)
{
	spi1_clock_enable();

	//enable RX path
	rf_rx_mode();

	//start rx
	cs_rf_active();
	spi1_trx(SX126X_SET_RX);			//command
	spi1_trx(RX_TIMEOUT_1500MS_2);		//1500 ms rx timeout for full packet (no header used) + additional SetLoRaSymbNumTimeout enabled
	spi1_trx(RX_TIMEOUT_1500MS_1);
	spi1_trx(RX_TIMEOUT_1500MS_0);
	cs_rf_inactive();

	spi1_clock_disable();

	return 1;
}



//sx126x get received packet
void rf_get_rx_packet(void)
{
	spi1_clock_enable();

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

	spi1_clock_disable();
}



void rf_set_standby_xosc(void)
{
	spi1_clock_enable();

	cs_rf_active();
	spi1_trx(SX126X_SIZE_SET_STANDBY);			//command
	spi1_trx(STDBY_XOSC);
	cs_rf_inactive();

	spi1_clock_disable();
}



//sx126x get status
uint8_t rf_get_status(void)
{
	spi1_clock_enable();

    uint8_t status;
    cs_rf_active();
    spi1_trx(SX126X_GET_STATUS);		//send command byte
    status = spi1_trx(SX126X_NOP);		//send nop, get response
    cs_rf_inactive();

    spi1_clock_disable();

    return status;
}



//sx126x get interrupt status
uint16_t rf_get_irq_status(void)
{
	spi1_clock_enable();

    uint16_t status_msb, status_lsb;
    cs_rf_active();
    spi1_trx(SX126X_GET_IRQ_STATUS);	//send command byte
    spi1_trx(SX126X_NOP);
    status_msb = spi1_trx(SX126X_NOP);		//get MSB
    status_lsb = spi1_trx(SX126X_NOP);		//get LSB
    cs_rf_inactive();

    spi1_clock_disable();

    return ((status_msb << 8) | status_lsb);
}



//sx126x clear interrupts
void rf_clear_irq(void)
{
	spi1_clock_enable();

    cs_rf_active();
    spi1_trx(SX126X_CLR_IRQ_STATUS);	//send command byte
    spi1_trx(IRQ_MASK_ALL);				//clear all interrupts
    spi1_trx(IRQ_MASK_ALL);
    cs_rf_inactive();

    spi1_clock_disable();
}



uint8_t *get_air_packet_tx(void)
{
	return &air_packet_tx[0];
}



uint8_t *get_air_packet_rx(void)
{
	return &air_packet_rx[0];
}



void rf_workaround_15_1(void)
{
    //workaround 15.1 Modulation Quality with 500 kHz LoRa Bandwidth
	//run before each packet TX
    uint8_t spec_reg = 0;

    spi1_clock_enable();

    cs_rf_active();
    spi1_trx(SX126X_READ_REGISTER);
    spi1_trx(0x08); //Specific reg 0x0889
    spi1_trx(0x89);
    spi1_trx(0);	//NOP
    spec_reg = spi1_trx(0);
    cs_rf_inactive();

    spec_reg |= 0x04;

    cs_rf_active();
    spi1_trx(SX126X_WRITE_REGISTER);
    spi1_trx(0x08); //Specific reg 0x0889
    spi1_trx(0x89);
    spi1_trx(spec_reg);
    cs_rf_inactive();

    spi1_clock_disable();
}



void rf_workaround_15_2(void)
{
    //workaround 15.2 Better Resistance of the SX1268 Tx to Antenna Mismatch
	//run one after power up
    uint8_t tx_clamp_reg = 0;

    spi1_clock_enable();

    cs_rf_active();
    spi1_trx(SX126X_READ_REGISTER);
    spi1_trx(0x08); //TxClampConfig 0x08D8
    spi1_trx(0xD8);
    spi1_trx(0);	//NOP
    tx_clamp_reg = spi1_trx(0);
    cs_rf_inactive();

    tx_clamp_reg |= 0x1E;

    cs_rf_active();
    spi1_trx(SX126X_WRITE_REGISTER);
    spi1_trx(0x08); //TxClampConfig 0x08D8
    spi1_trx(0xD8);
    spi1_trx(tx_clamp_reg);
    cs_rf_inactive();

    spi1_clock_disable();
}



void rf_workaround_15_3(void)
{
    //workaround 15.3 Implicit Header Mode Timeout Behavior
	//run after every RxDone if RX timeout was active
    uint8_t tmp_reg = 0;

    spi1_clock_enable();

    //stop broken counter
    cs_rf_active();
    spi1_trx(SX126X_WRITE_REGISTER);
    spi1_trx(0x09); //0x0902
    spi1_trx(0x02);
    spi1_trx(tmp_reg);
    cs_rf_inactive();

    //clear potential broken even
    cs_rf_active();
    spi1_trx(SX126X_READ_REGISTER);
    spi1_trx(0x09); //0x0944
    spi1_trx(0x44);
    spi1_trx(0);	//NOP
    tmp_reg = spi1_trx(0);
    cs_rf_inactive();

    tmp_reg |= 0x02;

    cs_rf_active();
    spi1_trx(SX126X_WRITE_REGISTER);
    spi1_trx(0x09); //0x0944
    spi1_trx(0x44);
    spi1_trx(tmp_reg);
    cs_rf_inactive();

    spi1_clock_disable();
}



void rf_workaround_15_4(void)
{
    //workaround 15.4 Optimizing the Inverted IQ Operation
	//run once upon configuration
    uint8_t iq_reg = 0;

    spi1_clock_enable();

    cs_rf_active();
    spi1_trx(SX126X_READ_REGISTER);
    spi1_trx(0x07); //IQ reg 0x0736
    spi1_trx(0x36);
    spi1_trx(0);	//NOP
    iq_reg = spi1_trx(0);
    cs_rf_inactive();

    iq_reg |= 0x04;

    cs_rf_active();
    spi1_trx(SX126X_WRITE_REGISTER);
    spi1_trx(0x07); //IQ reg 0x0736
    spi1_trx(0x36);
    spi1_trx(iq_reg);
    cs_rf_inactive();

    spi1_clock_disable();
}



int8_t rf_get_last_snr(void)
{
	uint8_t last_snr = 0;

	spi1_clock_enable();

    cs_rf_active();
    spi1_trx(SX126X_GET_PKT_STATUS);
    spi1_trx(0); 				//NOP
    spi1_trx(0);				//RssiPkt
    last_snr = spi1_trx(0);	//SnrPkt
    spi1_trx(0);				//SignalRssiPkt
    cs_rf_inactive();

    spi1_clock_disable();

    return (last_snr / 4);
}



void rf_set_cw_tx(void)
{
	spi1_clock_enable();

	cs_rf_active();
	spi1_trx(SX126X_SET_TX_CONTINUOUS_WAVE);			//command
	cs_rf_inactive();

	spi1_clock_disable();
}



void rf_tx_power_test(void)
{
	while (1)
	{
		//set rf switch
		rf_tx_mode();

		// 1
		//wait
		delay_cyc(1000000);

		led_green_off();
		led_red_off();
		//set tx params
		rf_config_tx_power(-9);

		//set CW
		rf_set_cw_tx();

		//wait
		delay_cyc(1000000);

		//off
		rf_set_standby_xosc();



		// 2
		led_green_on();
		led_red_off();
		//set tx params
		rf_config_tx_power(0);

		//set CW
		rf_set_cw_tx();

		//wait
		delay_cyc(1000000);

		//off
		rf_set_standby_xosc();



		// 3
		led_green_off();
		led_red_on();
		//set tx params
		rf_config_tx_power(10);

		//set CW
		rf_set_cw_tx();

		//wait
		delay_cyc(1000000);

		//off
		rf_set_standby_xosc();



		// 3
		led_green_on();
		led_red_on();
		//set tx params
		rf_config_tx_power(20);

		//set CW
		rf_set_cw_tx();

		//wait
		delay_cyc(1000000);

		//off
		rf_set_standby_xosc();
	}
}
