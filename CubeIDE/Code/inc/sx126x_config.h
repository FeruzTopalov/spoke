/*
	SPOKE
    
    file: sx126x_config.h
*/



#ifndef SX126X_CONFIG_HEADER
#define SX126X_CONFIG_HEADER



//COMMANDS
// Operational Modes Functions
#define		SX126X_SET_SLEEP                  (0x84)
#define		SX126X_SET_STANDBY                (0x80)
#define		SX126X_SET_FS                     (0xC1)
#define		SX126X_SET_TX                     (0x83)
#define		SX126X_SET_RX                     (0x82)
#define		SX126X_SET_STOP_TIMER_ON_PREAMBLE (0x9F)
#define		SX126X_SET_RX_DUTY_CYCLE          (0x94)
#define		SX126X_SET_CAD                    (0xC5)
#define		SX126X_SET_TX_CONTINUOUS_WAVE     (0xD1)
#define		SX126X_SET_TX_INFINITE_PREAMBLE   (0xD2)
#define		SX126X_SET_REGULATOR_MODE         (0x96)
#define		SX126X_CALIBRATE                  (0x89)
#define		SX126X_CALIBRATE_IMAGE            (0x98)
#define		SX126X_SET_PA_CFG                 (0x95)
#define		SX126X_SET_RX_TX_FALLBACK_MODE    (0x93)
// Registers and buffer Access
#define		SX126X_WRITE_REGISTER (0x0D)
#define		SX126X_READ_REGISTER  (0x1D)
#define		SX126X_WRITE_BUFFER   (0x0E)
#define		SX126X_READ_BUFFER    (0x1E)
// DIO and IRQ Control Functions
#define		SX126X_SET_DIO_IRQ_PARAMS         (0x08)
#define		SX126X_GET_IRQ_STATUS             (0x12)
#define		SX126X_CLR_IRQ_STATUS             (0x02)
#define		SX126X_SET_DIO2_AS_RF_SWITCH_CTRL (0x9D)
#define		SX126X_SET_DIO3_AS_TCXO_CTRL      (0x97)
// RF Modulation and Packet-Related Functions
#define		SX126X_SET_RF_FREQUENCY          (0x86)
#define		SX126X_SET_PKT_TYPE              (0x8A)
#define		SX126X_GET_PKT_TYPE              (0x11)
#define		SX126X_SET_TX_PARAMS             (0x8E)
#define		SX126X_SET_MODULATION_PARAMS     (0x8B)
#define		SX126X_SET_PKT_PARAMS            (0x8C)
#define		SX126X_SET_CAD_PARAMS            (0x88)
#define		SX126X_SET_BUFFER_BASE_ADDRESS   (0x8F)
#define		SX126X_SET_LORA_SYMB_NUM_TIMEOUT (0xA0)
// Communication Status Information
#define		SX126X_GET_STATUS           (0xC0)
#define		SX126X_GET_RX_BUFFER_STATUS (0x13)
#define		SX126X_GET_PKT_STATUS       (0x14)
#define		SX126X_GET_RSSI_INST        (0x15)
#define		SX126X_GET_STATS            (0x10)
#define		SX126X_RESET_STATS          (0x00)
// Miscellaneous
#define		SX126X_GET_DEVICE_ERRORS (0x17)
#define		SX126X_CLR_DEVICE_ERRORS (0x07)



//COMMANDS ARG SIZE
// Operational Modes Functions
#define		SX126X_SIZE_SET_SLEEP                  (2)
#define		SX126X_SIZE_SET_STANDBY                (2)
#define		SX126X_SIZE_SET_FS                     (1)
#define		SX126X_SIZE_SET_TX                     (4)
#define		SX126X_SIZE_SET_RX                     (4)
#define		SX126X_SIZE_SET_STOP_TIMER_ON_PREAMBLE (2)
#define		SX126X_SIZE_SET_RX_DUTY_CYCLE          (7)
#define		SX126X_SIZE_SET_CAD                    (1)
#define		SX126X_SIZE_SET_TX_CONTINUOUS_WAVE     (1)
#define		SX126X_SIZE_SET_TX_INFINITE_PREAMBLE   (1)
#define		SX126X_SIZE_SET_REGULATOR_MODE         (2)
#define		SX126X_SIZE_CALIBRATE                  (2)
#define		SX126X_SIZE_CALIBRATE_IMAGE            (3)
#define		SX126X_SIZE_SET_PA_CFG                 (5)
#define		SX126X_SIZE_SET_RX_TX_FALLBACK_MODE    (2)
// Registers and buffer Access
// Full size: this value plus buffer size
#define		SX126X_SIZE_WRITE_REGISTER (3)
// Full size: this value plus buffer size
#define		SX126X_SIZE_READ_REGISTER (4)
// Full size: this value plus buffer size
#define		SX126X_SIZE_WRITE_BUFFER (2)
// Full size: this value plus buffer size
#define		SX126X_SIZE_READ_BUFFER (3)
// DIO and IRQ Control Functions
#define		SX126X_SIZE_SET_DIO_IRQ_PARAMS         (9)
#define		SX126X_SIZE_GET_IRQ_STATUS             (2)
#define		SX126X_SIZE_CLR_IRQ_STATUS             (3)
#define		SX126X_SIZE_SET_DIO2_AS_RF_SWITCH_CTRL (2)
#define		SX126X_SIZE_SET_DIO3_AS_TCXO_CTRL      (5)
// RF Modulation and Packet-Related Functions
#define		SX126X_SIZE_SET_RF_FREQUENCY           (5)
#define		SX126X_SIZE_SET_PKT_TYPE               (2)
#define		SX126X_SIZE_GET_PKT_TYPE               (2)
#define		SX126X_SIZE_SET_TX_PARAMS              (3)
#define		SX126X_SIZE_SET_MODULATION_PARAMS_GFSK (9)
#define		SX126X_SIZE_SET_MODULATION_PARAMS_LORA (5)
#define		SX126X_SIZE_SET_PKT_PARAMS_GFSK        (10)
#define		SX126X_SIZE_SET_PKT_PARAMS_LORA        (7)
#define		SX126X_SIZE_SET_CAD_PARAMS             (8)
#define		SX126X_SIZE_SET_BUFFER_BASE_ADDRESS    (3)
#define		SX126X_SIZE_SET_LORA_SYMB_NUM_TIMEOUT  (2)
// Communication Status Information
#define		SX126X_SIZE_GET_STATUS           (1)
#define		SX126X_SIZE_GET_RX_BUFFER_STATUS (2)
#define		SX126X_SIZE_GET_PKT_STATUS       (2)
#define		SX126X_SIZE_GET_RSSI_INST        (2)
#define		SX126X_SIZE_GET_STATS            (2)
#define		SX126X_SIZE_RESET_STATS          (7)
// Miscellaneous
#define		SX126X_SIZE_GET_DEVICE_ERRORS (2)
#define		SX126X_SIZE_CLR_DEVICE_ERRORS (3)
#define		SX126X_SIZE_MAX_BUFFER        (255)
#define		SX126X_SIZE_DUMMY_BYTE        (1)




#endif /*SX126X_CONFIG_HEADER*/
