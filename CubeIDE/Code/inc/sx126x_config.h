/*
	SPOKE
    
    file: sx126x_config.h
*/



#ifndef SX126X_CONFIG_HEADER
#define SX126X_CONFIG_HEADER


			//Size (Command + Arguments), Command, Argument1, Argument2, ...
#define 	SX126X_CONFIG_ARRAY {\
			SX126X_SIZE_CLR_DEVICE_ERRORS, SX126X_CLR_DEVICE_ERRORS, SX126X_NOP, SX126X_NOP, \
			SX126X_SIZE_SET_STANDBY, SX126X_SET_STANDBY, STDBY_RC, \
			SX126X_SIZE_SET_DIO2_AS_RF_SWITCH_CTRL, SX126X_SET_DIO2_AS_RF_SWITCH_CTRL, DIO2_AS_RF_ENABLED, \
			SX126X_SIZE_SET_DIO3_AS_TCXO_CTRL, SX126X_SET_DIO3_AS_TCXO_CTRL, TCXO_CTRL_1_8V, TCXO_DELAY_5MS_23_16, TCXO_DELAY_5MS_15_8, TCXO_DELAY_5MS_7_0, \
			SX126X_SIZE_SET_REGULATOR_MODE, SX126X_SET_REGULATOR_MODE, USE_DCDC, \
			SX126X_SIZE_CALIBRATE, SX126X_CALIBRATE, CALIBRATE_ALL, \
			SX126X_SIZE_CALIBRATE_IMAGE, SX126X_CALIBRATE_IMAGE, CALIBRATE_IMAGE_FREQ1_430M, CALIBRATE_IMAGE_FREQ2_440M, \
			SX126X_SIZE_SET_PKT_TYPE, SX126X_SET_PKT_TYPE, PACKET_TYPE_LORA, \
			SX126X_SIZE_SET_RF_FREQUENCY, SX126X_SET_RF_FREQUENCY, RF_FREQ_LPD_CH1_31_24, RF_FREQ_LPD_CH1_23_16, RF_FREQ_LPD_CH1_15_8, RF_FREQ_LPD_CH1_7_0, \
			SX126X_SIZE_SET_PA_CFG, SX126X_SET_PA_CFG, PA_CFG_DUTY_CYCLE_22DB, PA_CFG_HP_MAX_22DB, PA_CFG_DEV_SEL, PA_CFG_LUT, \
			SX126X_SIZE_SET_TX_PARAMS, SX126X_SET_TX_PARAMS, TX_POWER_NEG9DB, TX_RAMP_TIME_800U, \
			SX126X_SIZE_SET_BUFFER_BASE_ADDRESS, SX126X_SET_BUFFER_BASE_ADDRESS, BASE_ADDR_TX, BASE_ADDR_RX, \
			SX126X_SIZE_SET_MODULATION_PARAMS_LORA, SX126X_SET_MODULATION_PARAMS, SX126X_LORA_SF12, SX126X_LORA_BW_125, SX126X_LORA_CR_4_8, SX126X_LORA_LDROPT_OFF, \
			SX126X_SIZE_SET_PKT_PARAMS_LORA, SX126X_SET_PKT_PARAMS, LORA_PP1_PREAMB_10_SYM, LORA_PP2_PREAMB_10_SYM, LORA_PP3_HEADER_IMPLCT, LORA_PP4_PLOAD_LEN_12_BYTE, LORA_PP5_CRC_ON, LORA_PP6_IQ_STD, \
			SX126X_SIZE_SET_LORA_SYMB_NUM_TIMEOUT, SX126X_SET_LORA_SYMB_NUM_TIMEOUT, LORA_SYMB_TIMEOUT_5_SYM, \
			SX126X_SIZE_SET_DIO_IRQ_PARAMS, SX126X_SET_DIO_IRQ_PARAMS, IRQ_MASK_1, IRQ_MASK_0, IRQ_DIO1_MASK_1, IRQ_DIO1_MASK_0, IRQ_DIO2_MASK_1, IRQ_DIO2_MASK_0, IRQ_DIO3_MASK_1, IRQ_DIO3_MASK_0, \
			SX126X_SIZE_SET_RX_TX_FALLBACK_MODE, SX126X_SET_RX_TX_FALLBACK_MODE, FALLBACK_MODE_RC, \
			SX126X_SIZE_SET_STANDBY, SX126X_SET_STANDBY, STDBY_RC, \
			0x00  			/* end of the sequence */ \
			}



#endif /*SX126X_CONFIG_HEADER*/
