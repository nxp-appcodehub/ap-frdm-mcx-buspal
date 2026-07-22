/*
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
 * Copyright 2024 NXP
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "bus_pal_hardware.h"


////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////
/*!
 * @brief init uart functions.
 */
static void init_uart(uint8_t instance);

/*!
 * @brief dspi initialization.
 */
static void init_spi(uint8_t instance);

/*!
 * @brief i2c initialization.
 */
static void init_i2c(uint8_t instance);

/*!
 * @brief i2c de-initialization.
 */
static void deinit_i2c(uint8_t instance);

/*!
 * @brief flexcan initialization.
 */
static void init_flexcan(uint8_t);

static status_t FLEXCAN_Send(uint8_t instance,
                             uint32_t mb_idx,
                             flexcan_data_info_t *tx_info,
                             uint32_t msg_id,
                             uint8_t *mb_data,
                             uint32_t timeout_ms);

/*!
 * @brief uart rx callback function.
 */
static void uart_rx_callback(uint8_t byte);

/*!
 * @brief flexcan callback function.
 */
void FLEXCAN_IRQHandler(uint8_t instance);



////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

//! @brief Variable for i2c host configuration information
static i2c_user_config_t s_i2cUserConfig = {.slaveAddress = 0x10, //!< The slave's 7-bit address
                                            .baudRate_kbps = 100 };

static spi_user_config_t s_spiUserConfig = {.polarity 		= kLPSPI_ClockPolarityActiveLow, 	/*!< Clock polarity */
                                            .phase 			= kLPSPI_ClockPhaseSecondEdge,    	/*!< Clock phase */
                                            .direction 		= kLPSPI_MsbFirst,              	/*!< MSB or LSB */
                                            .baudRate_Bps 	= 100000,                  			/*!< Baud Rate for SPI in Hz */
                                            .clock_Hz 		= 0 };

//! @brief Global state for the FlexCAN peripheral interface.
flexcan_transfer_info_t s_flexcanInfo;

//! @brief Variable for host data receiving
static uint8_t *s_rxData;
static uint32_t s_bytesRx;


static const clock_ip_name_t 	s_lpuart_clocks[] 	= LPUART_CLOCKS;
static const IRQn_Type			s_lpuartIRQs[]		= LPUART_RX_TX_IRQS;

static const clock_ip_name_t 	s_lpi2c_clocks[] 	= LPI2C_CLOCKS;
static const clock_ip_name_t 	s_lpspi_clocks[] 	= LPSPI_CLOCKS;
static const clock_ip_name_t 	s_flexcan_clocks[] 	= FLEXCAN_CLOCKS;

static const uint32_t 			g_lpuartBaseAddr[] 	= LPUART_BASE_ADDRS;
static const uint32_t 			g_lpi2cBaseAddr[] 	= LPI2C_BASE_ADDRS;
static const uint32_t 			g_lpspiBaseAddr[] 	= LPSPI_BASE_ADDRS;
static const uint32_t 			g_canBaseAddr[]		= CAN_BASE_ADDRS;


/* Tables to save CAN IRQ enum numbers defined in CMSIS header file. */
const IRQn_Type g_flexcanRxWarningIrqId[] = CAN_Rx_Warning_IRQS;
const IRQn_Type g_flexcanTxWarningIrqId[] = CAN_Tx_Warning_IRQS;
const IRQn_Type g_flexcanWakeUpIrqId[] = CAN_Wake_Up_IRQS;
const IRQn_Type g_flexcanErrorIrqId[] = CAN_Error_IRQS;
const IRQn_Type g_flexcanBusOffIrqId[] = CAN_Bus_Off_IRQS;
const IRQn_Type g_flexcanOredMessageBufferIrqId[] = CAN_ORed_Message_buffer_IRQS;



const uint32_t bit_rate_table[] = {
    125000, /* 125 kHz */
    250000, /* 250 kHz */
    500000, /* 500 kHz */
    750000, /* 750 kHz */
    1000000  /* 1   MHz */
};


void init_hardware(void)
{
	/* Start the microseconds timer (SysTick). This must be done before any
	 * code relies on microseconds_get_ticks(), for example the CAN receive
	 * timeout in read_can_data(). Without it SysTick never runs, the tick
	 * value never changes, and the CAN read loop can never time out, which
	 * causes the firmware to hang when the slave does not reply. */
	microseconds_init();

	/* Init peripherals*/
	init_uart(UART_HOST);
	init_spi(LPSPI_INSTANCE);
	init_i2c(I2C_INSTANCE);
	init_flexcan(FLEXCAN_INSTANCE);

}

/*FUNCTION**********************************************************************
 *
 * Function Name : init_uart
 * Description   : Initialize UART ports
 *
 *END**************************************************************************/
static void init_uart(uint8_t instance)
{
    uint32_t base = g_lpuartBaseAddr[instance];
	lpuart_config_t config;

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClockDiv(kCLOCK_DivLPUART0, 1u);
    CLOCK_AttachClk(kFRO12M_to_LPUART0);

    RESET_PeripheralReset(kLPUART0_RST_SHIFT_RSTn);

    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = 57600;
    config.enableRx = true;
    config.enableTx = true;

    LPUART_Init((LPUART_Type*)base, &config, 12000000U);
    LPUART_EnableInterrupts((LPUART_Type*)base, kLPUART_RxDataRegFullInterruptEnable);
    NVIC_EnableIRQ(s_lpuartIRQs[instance]);
}


/********************************************************************/
/*
 * LPUART1 IRQ Handler
 *
 */
void LPUART0_IRQHandler(void)
{
	 uart_rx_callback( (uint8_t)LPUART0->DATA );
}

void CAN0_IRQHandler(void)
{
	FLEXCAN_IRQHandler(0);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : uart_rx_callback
 * Description   : uart callback function
 *
 *END**************************************************************************/
void uart_rx_callback(uint8_t byte)
{
    if (s_rxData)
    {
        s_rxData[s_bytesRx++] = byte;
    }
}


/*FUNCTION**********************************************************************
 *
 * Function Name : write_bytes_to_host
 * Description   : sending host bytes command process
 *
 *END**************************************************************************/
void write_bytes_to_host(uint8_t * src, uint32_t lenght)
{
	LPUART_WriteBlocking((LPUART_Type*)g_lpuartBaseAddr[UART_HOST], src, lenght);
}


/*FUNCTION**********************************************************************
 *
 * Function Name : get_bytes_received_from_host
 * Description   : receiving host get bytes command process
 *
 *END**************************************************************************/
uint32_t get_bytes_received_from_host(void)
{
    return s_bytesRx;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : host_start_command_rx
 * Description   : receiving host start command process
 *
 *END**************************************************************************/
void host_start_command_rx(uint8_t *dest)
{
    s_rxData = dest;
    s_bytesRx = 0;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : host_stop_command_rx
 * Description   : receiving host stop command process
 *
 *END**************************************************************************/
void host_stop_command_rx(void)
{
    s_rxData = 0;
    s_bytesRx = 0;
}

static void init_spi(uint8_t instance)
{
	lpspi_master_config_t config;
	uint32_t baseAddr = g_lpspiBaseAddr[instance];

    /* Attach peripheral clock */
    CLOCK_SetClockDiv(kCLOCK_DivLPSPI0, 1u);
    CLOCK_AttachClk(kFRO12M_to_LPSPI0);

	LPSPI_MasterGetDefaultConfig(&config);

	config.cpol 				= s_spiUserConfig.polarity;
	config.cpha 				= s_spiUserConfig.phase;
	config.baudRate 			= s_spiUserConfig.baudRate_Bps;
	s_spiUserConfig.clock_Hz 	= CLOCK_GetLpspiClkFreq(0);

	LPSPI_MasterInit((LPSPI_Type *)baseAddr, &config, s_spiUserConfig.clock_Hz);
}

void send_spi_data(uint8_t *src, uint32_t writeLength)
{
    lpspi_transfer_t send_data = {0};
    uint32_t baseAddr = g_lpspiBaseAddr[LPSPI_INSTANCE];

    send_data.txData = src;
    send_data.dataSize = writeLength;
    send_data.rxData = NULL;
    send_data.configFlags = LPSPI_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous;

    LPSPI_MasterTransferBlocking((LPSPI_Type *)baseAddr, &send_data);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : receive_spi_data
 * Description   : spi receiving data process
 *
 *END**************************************************************************/
void receive_spi_data(uint8_t *dest, uint32_t readLength)
{
    lpspi_transfer_t receive_data = {0};
    uint32_t baseAddr = g_lpspiBaseAddr[LPSPI_INSTANCE];

    receive_data.rxData = dest;
    receive_data.dataSize = readLength;
    receive_data.txData = NULL;
    receive_data.configFlags = LPSPI_PCS_FOR_TRANSFER | kLPSPI_MasterPcsContinuous;

    LPSPI_MasterTransferBlocking((LPSPI_Type *)baseAddr, &receive_data);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : configure_spi_speed
 * Description   : spi config speed process
 *
 *END**************************************************************************/
void configure_spi_speed(uint32_t speedkhz)
{
    s_spiUserConfig.baudRate_Bps = speedkhz * 1000;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : configure_spi_settings
 * Description   : spi config settings process
 *
 *END**************************************************************************/
void configure_spi_settings(lpspi_clock_polarity_t polarity,
							lpspi_clock_phase_t phase,
							lpspi_shift_direction_t direction)
{
    s_spiUserConfig.polarity = polarity;
    s_spiUserConfig.phase = phase;
    s_spiUserConfig.direction = direction;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : init_flexcan
 * Description   : Initialize flexCAN
 *
 *END**************************************************************************/
static void init_flexcan(uint8_t instance)
{
    flexcan_config_t config;
    flexcan_timing_config_t timing_config;
    uint32_t baseAddr = g_canBaseAddr[instance];

    /* Initialize board hardware. */
    /* Attach peripheral clock */
    CLOCK_SetClockDiv(kCLOCK_DivFLEXCAN0, 1U);
    CLOCK_SetClockDiv(kCLOCK_DivFRO_HF_DIV, 1U);
    CLOCK_AttachClk(kFRO_HF_DIV_to_FLEXCAN0);

    FLEXCAN_GetDefaultConfig(&config);

    config.maxMbNum = 16;
    config.bitRate = DEFAULT_CAN_SPEED;

    /* Select mailbox number */
    s_flexcanInfo.rx_mailbox_num = 8;
    s_flexcanInfo.tx_mailbox_num = 9;

    /* make bootloader as node 'b' for testing */
    s_flexcanInfo.rxId = 0x123;
    s_flexcanInfo.txId = 0x321;

    s_flexcanInfo.rx_buf_write_index = 0;
    s_flexcanInfo.rx_buf_read_index = 0;

    /* Init the interrupt sync object.*/
    OSA_SemaCreate(&s_flexcanInfo.state.txIrqSync, 0);
    OSA_SemaCreate(&s_flexcanInfo.state.rxIrqSync, 0);
    s_flexcanInfo.state.rx_mb_idx = 8;
    s_flexcanInfo.state.tx_mb_idx = 9;

    // also need to get clock selection config data
    FLEXCAN_Init((CAN_Type *)baseAddr, &config, CLOCK_GetFlexcanClkFreq());

    FLEXCAN_Enable((CAN_Type *)baseAddr, true);

    FLEXCAN_EnableInterrupts((CAN_Type *)baseAddr, kFLEXCAN_ErrorInterruptEnable);

    // using setting table
    if(true == FLEXCAN_CalculateImprovedTimingValues((CAN_Type *)baseAddr, config.bitRate, CLOCK_GetFlexcanClkFreq(), &timing_config))
    {
        FLEXCAN_SetTimingConfig((CAN_Type *)baseAddr, &timing_config);
    }
    
    // FlexCAN reveive config
    s_flexcanInfo.rx_info.msg_id_type = kFLEXCAN_FrameFormatStandard;
    s_flexcanInfo.rx_info.data_length = 8;

    // Configure RX MB fields
    flexcan_rx_mb_config_t mbConfig;
    mbConfig.format = kFLEXCAN_FrameFormatStandard;
    mbConfig.id = CAN_ID_STD(s_flexcanInfo.rxId);
    mbConfig.type = kFLEXCAN_FrameTypeData;
    FLEXCAN_SetRxMbConfig((CAN_Type *)baseAddr, 8, &mbConfig, true);

    //Enable Message Buffer Interrupt
    FLEXCAN_EnableMbInterrupts((CAN_Type *)baseAddr, 1 << 8);
    // FlexCAN transfer config
    s_flexcanInfo.tx_info.msg_id_type = kFLEXCAN_FrameFormatStandard;
    s_flexcanInfo.tx_info.data_length = 8;

    FLEXCAN_SetTxMbConfig((CAN_Type *)baseAddr, 9, false);

    NVIC_EnableIRQ(g_flexcanErrorIrqId[0]);
    NVIC_EnableIRQ(g_flexcanBusOffIrqId[0]);
    NVIC_EnableIRQ(g_flexcanOredMessageBufferIrqId[0]);
}


/*FUNCTION**********************************************************************
 *
 * Function Name : configure_can_txid
 * Description   : flexCAN config tx id
 *
 *END**************************************************************************/
void configure_can_txid(uint32_t txid)
{
    s_flexcanInfo.txId = txid & 0x7ff; // support 11 bit std id
}

/*FUNCTION**********************************************************************
 *
 * Function Name : configure_can_rxid
 * Description   : flexCAN config rx id
 *
 *END**************************************************************************/
void configure_can_rxid(uint32_t rxid)
{
    uint32_t baseAddr = g_canBaseAddr[FLEXCAN_INSTANCE];
    s_flexcanInfo.rxId = rxid & 0x7ff; // support 11 bit std id

    // Configure RX MB fields
    flexcan_rx_mb_config_t mbConfig;
    mbConfig.format = kFLEXCAN_FrameFormatStandard;
    mbConfig.id = CAN_ID_STD(s_flexcanInfo.rxId);
    mbConfig.type = kFLEXCAN_FrameTypeData;
    FLEXCAN_SetRxMbConfig((CAN_Type *)baseAddr, 8, &mbConfig, true);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : configure_can_speed
 * Description   : flexCAN config speed process
 *
 *END**************************************************************************/
void configure_can_speed(uint32_t speed)
{
    uint32_t baseAddr = g_canBaseAddr[FLEXCAN_INSTANCE];
    if (speed < 5)
    {
    	// using setting table
    	FLEXCAN_SetBitRate((CAN_Type *)baseAddr, CLOCK_GetFreq(kCLOCK_FroHfDiv), bit_rate_table[speed]);
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : send_can_data
 * Description   :  flexCAN sending data process
 *
 *END**************************************************************************/
void send_can_data(uint8_t *src, uint32_t writeLength)
{
    uint32_t sentCnt = 0;
    uint8_t *sendPtr = src;

    while (sentCnt < writeLength)
    {
        if ((writeLength - sentCnt) <= 8)
        {
            // number of bytes to be sent
            s_flexcanInfo.tx_info.data_length = writeLength - sentCnt;
            sentCnt += writeLength - sentCnt;
        }
        else
        {
            // number of bytes to be sent
            s_flexcanInfo.tx_info.data_length = 8;
            sentCnt += 8;
        }

        FLEXCAN_Send(0, 9, &s_flexcanInfo.tx_info, s_flexcanInfo.txId, (uint8_t *) sendPtr, 1000);

        sendPtr += s_flexcanInfo.tx_info.data_length;
    }

    reset_can_buffer();
}

status_t FLEXCAN_Send(uint8_t instance,
                      uint32_t mb_idx,
                      flexcan_data_info_t *tx_info,
                      uint32_t msg_id,
                      uint8_t *mb_data,
                      uint32_t timeout_ms)
{
    uint32_t baseAddr = g_canBaseAddr[instance];
    osa_status_t syncStatus = kStatus_Success;
    uint8_t i;
    flexcan_frame_t frame;
    flexcan_mb_transfer_t xfer;
    xfer.mbIdx = mb_idx;
    frame.format = tx_info->msg_id_type;
    frame.length = tx_info->data_length;
    frame.id = CAN_ID_STD(msg_id);
    frame.type = kFLEXCAN_FrameTypeData;
    xfer.frame = &frame;


    /* Copy user's buffer into the message buffer data area*/
    if (mb_data != NULL)
    {
        xfer.frame->dataWord0 = 0x0;
        xfer.frame->dataWord1 = 0x0;

        for (i = 0; i < tx_info->data_length; i++)
        {
            uint32_t temp, temp1;
            temp1 = (*(mb_data + i));
            if (i < 4)
            {
                temp = temp1 << ((3 - i) * 8);
                xfer.frame->dataWord0 |= temp;
            }
            else
            {
                temp = temp1 << ((7 - i) * 8);
                xfer.frame->dataWord1 |= temp;
            }
        }
    }

    if (kStatus_Success == FLEXCAN_WriteTxMb((CAN_Type *)baseAddr, mb_idx, xfer.frame))
    {
        /* Enable Message Buffer Interrupt. */
        FLEXCAN_EnableMbInterrupts((CAN_Type *)baseAddr, 1 << mb_idx);

        do
        {
            syncStatus = OSA_SemaWait(&s_flexcanInfo.state.txIrqSync, timeout_ms);
        } while (syncStatus == kStatus_OSA_Idle);

        /* Disable message buffer interrupt*/
        FLEXCAN_DisableMbInterrupts((CAN_Type *)baseAddr, 1 << mb_idx);

        /* Wait for the interrupt*/
        if (syncStatus != kStatus_OSA_Success)
        {
            return kStatus_Timeout;
        }
        else
        {
            return kStatus_Success;
        }
    }
    else
    {
        return kStatus_Fail;
    }
}

void FLEXCAN_IRQHandler(uint8_t instance)
{
    volatile uint32_t flag_reg;
    uint32_t temp;
    CAN_Type *baseAddr = (CAN_Type *)g_canBaseAddr[instance];

    /* Get the interrupts that are enabled and ready */
    flag_reg = (baseAddr->IFLAG1 & baseAddr->IMASK1);

    /* Check Tx/Rx interrupt flag and clear the interrupt */
    if (flag_reg)
    {
        temp = (1 << s_flexcanInfo.state.rx_mb_idx);
        if (flag_reg & temp)
        {
            OSA_SemaPost(&s_flexcanInfo.state.rxIrqSync);

            flexcan_frame_t rxFrame;
            /* Get RX MB field values*/
            if (!FLEXCAN_ReadRxMb(baseAddr, s_flexcanInfo.state.rx_mb_idx, &rxFrame))
            {
                uint8_t i;
                uint8_t sink_byte = 0;
                for (i = 0; i < rxFrame.length; i++)
                {
                    switch (i)
                    {
                        case 0:
                            sink_byte = rxFrame.dataByte0;
                            break;
                        case 1:
                            sink_byte = rxFrame.dataByte1;
                            break;
                        case 2:
                            sink_byte = rxFrame.dataByte2;
                            break;
                        case 3:
                            sink_byte = rxFrame.dataByte3;
                            break;
                        case 4:
                            sink_byte = rxFrame.dataByte4;
                            break;
                        case 5:
                            sink_byte = rxFrame.dataByte5;
                            break;
                        case 6:
                            sink_byte = rxFrame.dataByte6;
                            break;
                        case 7:
                            sink_byte = rxFrame.dataByte7;
                            break;
                        default:
                            break;
                    }
                    receive_can_data(sink_byte, instance);
                }
            }
        }

        temp = (1 << s_flexcanInfo.state.tx_mb_idx);
        if (flag_reg & temp)
        {
            OSA_SemaPost(&s_flexcanInfo.state.txIrqSync);
        }

        baseAddr->IFLAG1 = flag_reg;
    }

    /* Clear all other interrupts in ERRSTAT register (Error, Busoff, Wakeup) */
    FLEXCAN_ClearStatusFlags(baseAddr, kFLEXCAN_ErrorFlag | CAN_ESR1_ERRINT_MASK);

    return;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : receive_can_data
 * Description   : flexCAN receiving data process
 *
 *END**************************************************************************/
void receive_can_data(uint8_t data, uint32_t instance)
{
    s_flexcanInfo.rx_buf[s_flexcanInfo.rx_buf_write_index++] = data;
    s_flexcanInfo.rx_buf_write_index &= 0x1FF;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : reset_can_buffer
 * Description   : flexCAN reset buffer process
 *
 *END**************************************************************************/
void reset_can_buffer(void)
{
    s_flexcanInfo.rx_buf_write_index = 0;
    s_flexcanInfo.rx_buf_read_index = 0;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : read_can_data
 * Description   : flexCAN read data process
 *
 *END**************************************************************************/
void read_can_data(uint8_t *dest, uint32_t readLength)
{
    uint32_t received_cnt = 0;
    uint64_t timeoutTicks = microseconds_get_ticks() + 20875 * 500; // 5ms time out

    while ((received_cnt < readLength) && (microseconds_get_ticks() < timeoutTicks))
    {
        if (s_flexcanInfo.rx_buf_read_index != s_flexcanInfo.rx_buf_write_index)
        {
            dest[received_cnt++] = s_flexcanInfo.rx_buf[s_flexcanInfo.rx_buf_read_index++];
            s_flexcanInfo.rx_buf_read_index &= 0x1FF;
        }
    }
}

/*FUNCTION**********************************************************************
 *
 * Function Name : init_i2c
 * Description   : I2C init function
 *
 *END**************************************************************************/
static void init_i2c(uint8_t instance)
{
    uint32_t baseAddr = g_lpi2cBaseAddr[instance];
    lpi2c_master_config_t config;

    /* Attach peripheral clock */
    CLOCK_SetClockDiv(kCLOCK_DivLPI2C3, 1u);
    CLOCK_AttachClk(kFRO12M_to_LPI2C3);

    LPI2C_MasterGetDefaultConfig(&config);

    LPI2C_MasterInit((LPI2C_Type *)baseAddr, &config, CLOCK_GetLpi2cClkFreq(3u));
}

/*FUNCTION**********************************************************************
 *
 * Function Name : deinit_i2c
 * Description   : I2C de-init function
 *
 *END**************************************************************************/
void deinit_i2c(uint8_t instance)
{
    uint32_t baseAddr = g_lpi2cBaseAddr[instance];
    LPI2C_MasterDeinit((LPI2C_Type *)baseAddr);
}

/*FUNCTION**********************************************************************
 *
 * Function Name : configure_i2c_address
 * Description   : i2c config address process
 *
 *END**************************************************************************/
void configure_i2c_address(uint8_t address)
{
    s_i2cUserConfig.slaveAddress = address;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : configure_i2c_speed
 * Description   : i2c config speed process
 *
 *END**************************************************************************/
void configure_i2c_speed(uint32_t speedkhz)
{
    s_i2cUserConfig.baudRate_kbps = speedkhz;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : send_i2c_data
 * Description   : i2c sending data process
 *
 *END**************************************************************************/
status_t send_i2c_data(uint8_t *src, uint32_t writeLength)
{
    lpi2c_master_transfer_t send_data = {0};
    uint32_t baseAddr 		= g_lpi2cBaseAddr[I2C_INSTANCE];

    send_data.slaveAddress 	= s_i2cUserConfig.slaveAddress;
    send_data.direction 	= kLPI2C_Write;
    send_data.data 			= src;
    send_data.dataSize		= writeLength;

    if (LPI2C_MasterTransferBlocking((LPI2C_Type *)baseAddr, &send_data) != kStatus_Success)
    {
        deinit_i2c(I2C_INSTANCE);
        init_i2c(I2C_INSTANCE);
        return kStatus_Fail;
    }

    return kStatus_Success;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : receive_i2c_data
 * Description   : i2c receiving data process
 *
 *END**************************************************************************/
status_t receive_i2c_data(uint8_t *dest, uint32_t readLength)
{
    lpi2c_master_transfer_t receive_data = {0};
    uint32_t baseAddr 			= g_lpi2cBaseAddr[I2C_INSTANCE];
    receive_data.slaveAddress 	= s_i2cUserConfig.slaveAddress;
    receive_data.direction 		= kLPI2C_Read;
    receive_data.data 			= dest;
    receive_data.dataSize 		= readLength;

    if (LPI2C_MasterTransferBlocking((LPI2C_Type *)baseAddr, &receive_data) != kStatus_Success)
    {
        deinit_i2c(I2C_INSTANCE);
        init_i2c(I2C_INSTANCE);
        return kStatus_Fail;
    }

    return kStatus_Success;
}


