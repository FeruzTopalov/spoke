/*
	SPOKE
    
    file: sx126x.h
*/



#ifndef SX126X_HEADER
#define SX126X_HEADER



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
#define		SX126X_NOP 					  (0)



//ARG PARAMETER/CONFIG
// SX126X_SET_STANDBY
#define		STDBY_RC		(0x00)
#define		STDBY_XOSC		(0x01)

// SX126X_SET_DIO2_AS_RF_SWITCH_CTRL
#define		DIO2_AS_RF_ENABLED		(0x01)
#define		DIO2_AS_RF_DISABLED		(0x00)

// SX126X_SET_DIO3_AS_TCXO_CTRL
#define		TCXO_CTRL_1_6V                          (0x00)
#define		TCXO_CTRL_1_7V                          (0x01)
#define		TCXO_CTRL_1_8V                          (0x02)
#define		TCXO_CTRL_2_2V                          (0x03)
#define		TCXO_CTRL_2_4V                          (0x04)
#define		TCXO_CTRL_2_7V                          (0x05)
#define		TCXO_CTRL_3_0V                          (0x06)
#define		TCXO_CTRL_3_3V                          (0x07)

#define		TCXO_DELAY_5MS_23_16					(0x00)
#define		TCXO_DELAY_5MS_15_8						(0x01)
#define		TCXO_DELAY_5MS_7_0						(0x40)

// SX126X_SET_REGULATOR_MODE
#define		USE_LDO 	(0x00)
#define		USE_DCDC	(0x01)

// SX126X_CALIBRATE
#define		CALIBRATE_ALL 	(0x7F)

// SX126X_CALIBRATE_IMAGE
#define		CALIBRATE_IMAGE_FREQ1_430M 	(0x6B)
#define		CALIBRATE_IMAGE_FREQ2_440M 	(0x6F)

// SX126X_SET_PKT_TYPE
#define		PACKET_TYPE_GFSK		(0x00)
#define		PACKET_TYPE_LORA		(0x01)

// SX126X_SET_RF_FREQUENCY
#define		RF_FREQ_LPD_CH1_31_24		(0x1B)
#define		RF_FREQ_LPD_CH1_23_16		(0x11)
#define		RF_FREQ_LPD_CH1_15_8		(0x33)
#define		RF_FREQ_LPD_CH1_7_0			(0x33)

// SX126X_SET_PA_CFG
#define		PA_CFG_DUTY_CYCLE_22DB		(0x04)	//+22 dBm PA
#define		PA_CFG_HP_MAX_22DB			(0x07)	//+22 dBm PA
#define		PA_CFG_DEV_SEL				(0x00)
#define		PA_CFG_LUT					(0x01)

// SX126X_SET_TX_PARAMS
#define		TX_POWER_10DB			(0x0A)	//+10 dBm power level
#define		TX_RAMP_TIME_800U		(0x05)	//800 us

// SX126X_SET_BUFFER_BASE_ADDRESS
#define		BASE_ADDR_TX				(0x00)
#define		BASE_ADDR_RX				(0x7F)

// SX126X_SET_MODULATION_PARAMS
#define		FSK_MP1_BR_1200				(0x0D)
#define		FSK_MP2_BR_1200				(0x05)
#define		FSK_MP3_BR_1200				(0x55)
#define		FSK_MP4_SHAPE_00			(0x00)	//Pure FSK
#define		FSK_MP4_SHAPE_05			(0x09)	//GFSK BT 0.5
#define		FSK_MP5_RX_BW_4800			(0x1F)
#define		FSK_MP5_RX_BW_11700			(0x16)
#define		FSK_MP6_FDEV_1200			(0x00)
#define		FSK_MP7_FDEV_1200			(0x04)
#define		FSK_MP8_FDEV_1200			(0xEA)

// SX126X_SET_PKT_PARAMS
#define		FSK_PP1_PREAMB_3_BYTE		(0x00)	//24 bits
#define		FSK_PP2_PREAMB_3_BYTE		(0x18)
#define		FSK_PP3_PREAMB_THS_1_BYTE	(0x04)
#define		FSK_PP4_SYNCW_LEN_2_BYTE	(0x10)	//16 bits
#define		FSK_PP5_ADDR_COMP_DIS		(0x00)
#define		FSK_PP6_PKT_TYPE_FIXED		(0x00)
#define		FSK_PP7_PLOAD_LEN_12_BYTE	(0x0C)	//payload len only, no syncword/crc included
#define		FSK_PP8_CRC_TYPE_2_BYTE		(0x02)
#define		FSK_PP9_WHITE_NO			(0x00)

// SET Sync Word
#define		SYNC_WORD_0		(0x4B)
#define		SYNC_WORD_1		(0xB2)

// SET CRC
#define 	CRC_IBM_SEED				(0xFFFF)
#define 	CRC_IBM_SEED_0				(0xFF)
#define 	CRC_IBM_SEED_1				(0xFF)
#define 	CRC_POLYNOMIAL_IBM			(0x8005)
#define 	CRC_POLYNOMIAL_IBM_0		(0x05)
#define 	CRC_POLYNOMIAL_IBM_1		(0x80)

// SX126X_SET_DIO_IRQ_PARAMS
#define		IRQ_TX_DONE						(0x0001)
#define		IRQ_RX_DONE						(0x0002)
#define		IRQ_PREAMBLE_DETECTED			(0x0004)
#define		IRQ_SYNCWORD_VALID				(0x0008)
#define		IRQ_HEADER_VALID				(0x0010)
#define		IRQ_HEADER_ERROR				(0x0020)
#define		IRQ_CRC_ERROR					(0x0040)
#define		IRQ_CAD_DONE					(0x0080)
#define		IRQ_CAD_ACTIVITY_DETECTED		(0x0100)
#define		IRQ_RX_TX_TIMEOUT				(0x0200)

#define		IRQ_MASK_ALL					(0xFF)
#define		IRQ_MASK						(IRQ_TX_DONE | IRQ_RX_DONE | IRQ_CRC_ERROR | IRQ_RX_TX_TIMEOUT)
#define		IRQ_DIO1_MASK					(IRQ_TX_DONE | IRQ_RX_DONE | IRQ_CRC_ERROR | IRQ_RX_TX_TIMEOUT)
#define		IRQ_DIO2_MASK					(0x0000)
#define		IRQ_DIO3_MASK					(0x0000)

#define		IRQ_MASK_1				((IRQ_MASK >> 8) & 0xFF)
#define		IRQ_MASK_0				(IRQ_MASK & 0xFF)
#define		IRQ_DIO1_MASK_1			((IRQ_DIO1_MASK >> 8) & 0xFF)
#define		IRQ_DIO1_MASK_0			(IRQ_DIO1_MASK & 0xFF)
#define		IRQ_DIO2_MASK_1			((IRQ_DIO2_MASK >> 8) & 0xFF)
#define		IRQ_DIO2_MASK_0			(IRQ_DIO2_MASK & 0xFF)
#define		IRQ_DIO3_MASK_1			((IRQ_DIO3_MASK >> 8) & 0xFF)
#define		IRQ_DIO3_MASK_0			(IRQ_DIO3_MASK & 0xFF)

// SX126X_SET_RX_TX_FALLBACK_MODE
#define		FALLBACK_MODE_XOSC				(0x30)

// SX126X_SET_TX
#define		TX_TIMEOUT_DISABLED_0				(0x00)
#define		TX_TIMEOUT_DISABLED_1				(0x00)
#define		TX_TIMEOUT_DISABLED_2				(0x00)

// SX126X_SET_RX
#define		RX_TIMEOUT_50MS_0				(0x80)	//3 bytes preamble + 2 bytes sync word = 40 bit; 40 / 1200 = ~33 ms; make 50 ms to ensure sync word reception
#define		RX_TIMEOUT_50MS_1				(0x0C)
#define		RX_TIMEOUT_50MS_2				(0x00)
#define		RX_TIMEOUT_DISABLED_0				(0x00)
#define		RX_TIMEOUT_DISABLED_1				(0x00)
#define		RX_TIMEOUT_DISABLED_2				(0x00)



//REGISTERS ADDRESS
#define SX126X_REG_ADDR_SYNC_WORD_0			(0x06C0)
#define SX126X_REG_ADDR_SYNC_WORD_0_0		(0xC0)
#define SX126X_REG_ADDR_SYNC_WORD_0_1		(0x06)

#define SX126X_REG_ADDR_CRC_INIT_VAL		(0x06BC)
#define SX126X_REG_ADDR_CRC_INIT_VAL_0		(0xBC)
#define SX126X_REG_ADDR_CRC_INIT_VAL_1		(0x06)






//REGISTERS ADDRESSES
//
///**
// * @brief The address of the register holding the first byte defining the CRC seed
// */
//#define SX126X_REG_CRCSEEDBASEADDRESS 0x06BC
//
///**
// * @brief The address of the register holding the first byte defining the CRC polynomial
// */
//#define SX126X_REG_CRCPOLYBASEADDRESS 0x06BE
//
///**
// * @brief The address of the register holding the first byte defining the whitening seed
// */
//#define SX126X_REG_WHITSEEDBASEADDRESS 0x06B8
//
///**
// * @brief The addresses of the registers holding SyncWords values
// */
//#define SX126X_REG_SYNCWORDBASEADDRESS 0x06C0
//
///**
// * @brief The addresses of the register holding LoRa Modem SyncWord value
// *        0x1424: LoRaWAN private network,
// *        0x3444: LoRaWAN public network
// */
//#define SX126X_REG_LR_SYNCWORD 0x0740
//
///**
// * @brief The address of the register holding the coding rate configuration extracted from a received LoRa header
// */
//#define SX126X_REG_LR_HEADER_CR 0x0749
//#define SX126X_REG_LR_HEADER_CR_POS ( 4U )
//#define SX126X_REG_LR_HEADER_CR_MASK ( 0x07UL << SX126X_REG_LR_HEADER_CR_POS )
//
///**
// * @brief The address of the register holding the CRC configuration extracted from a received LoRa header
// */
//#define SX126X_REG_LR_HEADER_CRC 0x076B
//#define SX126X_REG_LR_HEADER_CRC_POS ( 4U )
//#define SX126X_REG_LR_HEADER_CRC_MASK ( 0x01UL << SX126X_REG_LR_HEADER_CRC_POS )
//
///*!
// * The address of the register giving a 32-bit random number
// */
//#define SX126X_REG_RNGBASEADDRESS 0x0819
//
///*!
// * The address of the register used to disable the LNA
// */
//#define SX126X_REG_ANA_LNA 0x08E2
//
///*!
// * The address of the register used to disable the mixer
// */
//#define SX126X_REG_ANA_MIXER 0x08E5
//
///*!
// * The address of the register holding RX Gain value
// *     0x94: power saving,
// *     0x96: rx boosted
// */
//#define SX126X_REG_RXGAIN 0x08AC
//
///**
// * @brief Change the value on the device internal trimming capacitor
// */
//#define SX126X_REG_XTATRIM 0x0911
//
///**
// * @brief Set the current max value in the over current protection
// */
//#define SX126X_REG_OCP 0x08E7
//
///**
// * @brief WORKAROUND - Optimizing the Inverted IQ Operation, see DS_SX1261-2_V1.2 datasheet chapter 15.4
// */
//#define SX126X_REG_IQ_POLARITY 0x0736
//
///**
// * @brief WORKAROUND - Modulation Quality with 500 kHz LoRa Bandwidth, see DS_SX1261-2_V1.2 datasheet chapter 15.1
// */
//#define SX126X_REG_TX_MODULATION 0x0889
//
///**
// * @brief WORKAROUND - Better resistance to antenna mismatch, see DS_SX1261-2_V1.2 datasheet chapter 15.2
// */
//#define SX126X_REG_TX_CLAMP_CFG 0x08D8
//#define SX126X_REG_TX_CLAMP_CFG_POS ( 1U )
//#define SX126X_REG_TX_CLAMP_CFG_MASK ( 0x0FUL << SX126X_REG_TX_CLAMP_CFG_POS )
//
///**
// * @brief RTC control
// */
//#define SX126X_REG_RTC_CTRL 0x0902
//
///**
// * @brief Event clear
// */
//#define SX126X_REG_EVT_CLR 0x0944
//#define SX126X_REG_EVT_CLR_TIMEOUT_POS ( 1U )
//#define SX126X_REG_EVT_CLR_TIMEOUT_MASK ( 0x01UL << SX126X_REG_EVT_CLR_TIMEOUT_POS )
//
///**
// * @brief RX address pointer
// */
//#define SX126X_REG_RX_ADDRESS_POINTER 0x0803
//
///**
// * @brief RX/TX payload length
// */
//#define SX126X_REG_RXTX_PAYLOAD_LEN 0x06BB
//
///**
// * @brief Output disable
// */
//#define SX126X_REG_OUT_DIS_REG 0x0580
//#define SX126X_REG_OUT_DIS_REG_DIO3_POS ( 3U )
//#define SX126X_REG_OUT_DIS_REG_DIO3_MASK ( 0x01UL << SX126X_REG_OUT_DIS_REG_DIO3_POS )
//
///**
// * @brief Input enable
// */
//#define SX126X_REG_IN_EN_REG 0x0583
//#define SX126X_REG_IN_EN_REG_DIO3_POS ( 3U )
//#define SX126X_REG_IN_EN_REG_DIO3_MASK ( 0x01UL << SX126X_REG_IN_EN_REG_DIO3_POS )
//
///**
// * @brief TX bitbang A
// */
//#define SX126X_REG_BITBANG_A_REG 0x0680
//#define SX126X_REG_BITBANG_A_REG_ENABLE_POS ( 4U )
//#define SX126X_REG_BITBANG_A_REG_ENABLE_MASK ( 0x07UL << SX126X_REG_BITBANG_A_REG_ENABLE_POS )
//#define SX126X_REG_BITBANG_A_REG_ENABLE_VAL ( 0x01UL << SX126X_REG_BITBANG_A_REG_ENABLE_POS )
//
///**
// * @brief TX bitbang B
// */
//#define SX126X_REG_BITBANG_B_REG 0x0587
//#define SX126X_REG_BITBANG_B_REG_ENABLE_POS ( 0U )
//#define SX126X_REG_BITBANG_B_REG_ENABLE_MASK ( 0x0FUL << SX126X_REG_BITBANG_B_REG_ENABLE_POS )
//#define SX126X_REG_BITBANG_B_REG_ENABLE_VAL ( 0x0CUL << SX126X_REG_BITBANG_B_REG_ENABLE_POS )
//
///**
// * @brief Number of symbols given as SX126X_REG_LR_SYNCH_TIMEOUT[7:3] * 2 ^ (2*SX126X_REG_LR_SYNCH_TIMEOUT[2:0] + 1)
// */
//#define SX126X_REG_LR_SYNCH_TIMEOUT 0x0706
//
///**
// * @brief Base address of the register retention list
// */
//#define SX126X_REG_RETENTION_LIST_BASE_ADDRESS 0x029F



#endif /*SX126X_HEADER*/
