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

#ifndef BUS_PAL_HARDWARE_H_
#define BUS_PAL_HARDWARE_H_

#include "buspal_config.h"
#include "clock_config.h"
#include "fsl_clock.h"
#include "fsl_port.h"
#include "fsl_lpuart.h"
#include "fsl_lpi2c.h"
#include "fsl_lpspi.h"
#include "fsl_flexcan.h"
#include "fsl_rtos_abstraction.h"
#include "microseconds.h"

#define PORT_UART_RX 				_PORT_UART_RX
#define PIN_UART_RX 				_PIN_UART_RX 			
#define PORT_UART_TX				_PORT_UART_TX
#define PIN_UART_TX					_PIN_UART_TX
#define DEFAULT_UART_BAUDRATE		_DEFAULT_UART_BAUDRATE	
#define BUSPAL_UART_INSTANCE		_BUSPAL_UART_INSTANCE	

#define PORT_I2C_SCL				_PORT_I2C_SCL
#define PIN_I2C_SCL					_PIN_I2C_SCL			
#define PORT_I2C_SDA				_PORT_I2C_SDA
#define PIN_I2C_SDA					_PIN_I2C_SDA			
#define DEFAULT_I2C_SPEED			_DEFAULT_I2C_SPEED		
#define BUSPAL_I2C_INSTANCE			_BUSPAL_I2C_INSTANCE	

#define PORT_SPI_SDO				_PORT_SPI_SDO		
#define PIN_SPI_SDO					_PIN_SPI_SDO			
#define PORT_SPI_SDI				_PORT_SPI_SDI		
#define PIN_SPI_SDI					_PIN_SPI_SDI			
#define PORT_SPI_SCK				_PORT_SPI_SCK		
#define PIN_SPI_SCK					_PIN_SPI_SCK				
#define PORT_SPI_CS					_PORT_SPI_CS			
#define PIN_SPI_CS					_PIN_SPI_CS			
#define SPI_PCS						_SPI_PCS					
#define DEFAULT_SPI_SPEED			_DEFAULT_SPI_SPEED		
#define BUSPAL_SPI_INSTANCE			_BUSPAL_SPI_INSTANCE		

#define PORT_CAN_TX                 _PORT_CAN_TX
#define PIN_CAN_TX                  _PIN_CAN_TX             
#define PORT_CAN_RX                 _PORT_CAN_RX
#define PIN_CAN_RX                  _PIN_CAN_RX             
#define DEFAULT_CAN_SPEED           _DEFAULT_CAN_SPEED      
#define BUSPAL_CAN_INSTANCE			_BUSPAL_CAN_INSTANCE

#define UART_HOST 				    BUSPAL_UART_INSTANCE

#define I2C_INSTANCE 			    BUSPAL_I2C_INSTANCE

#define LPSPI_INSTANCE 			    BUSPAL_SPI_INSTANCE
#if defined(SPI_PCS) & SPI_PCS == 0
#define LPSPI_PCS_FOR_TRANSFER      kLPSPI_MasterPcs0
#elif defined(SPI_PCS) & SPI_PCS == 1
#define LPSPI_PCS_FOR_TRANSFER      kLPSPI_MasterPcs1
#elif defined(SPI_PCS) & SPI_PCS == 2
#define LPSPI_PCS_FOR_TRANSFER      kLPSPI_MasterPcs2
#elif defined(SPI_PCS) & SPI_PCS == 3
#define LPSPI_PCS_FOR_TRANSFER      kLPSPI_MasterPcs3
#endif

#define FLEXCAN_INSTANCE 		    BUSPAL_CAN_INSTANCE

typedef enum
{
	kPortA,
	kPortB,
	kPortC,
	kPortD
}_PortN;

/*!
 * @brief user config from host for i2c
 */
typedef struct _i2c_user_config
{
    uint8_t slaveAddress;
    uint16_t baudRate_kbps;
} i2c_user_config_t;

/*!
 * @brief user config from host for spi
 */
typedef struct _spi_user_config
{
    lpspi_clock_polarity_t polarity;   	/*!< Clock polarity */
    lpspi_clock_phase_t phase;         	/*!< Clock phase */
    lpspi_shift_direction_t direction;	/*!< MSB or LSB */
    uint32_t baudRate_Bps;           	/*!< Baud Rate for SPI in Hz */
    uint32_t clock_Hz;
} spi_user_config_t;


/*!
 * @brief Internal driver state information.
 *
 * @note The contents of this structure are internal to the driver and should not be
 *      modified by users. Also, contents of the structure are subject to change in
 *      future releases.
 */
typedef struct FlexCANState
{
    volatile uint32_t rx_mb_idx; /*!< Index of the message buffer for receiving*/
    volatile uint32_t tx_mb_idx; /*!< Index of the message buffer for transmitting*/
    semaphore_t txIrqSync;       /*!< Used to wait for ISR to complete its TX business.*/
    semaphore_t rxIrqSync;       /*!< Used to wait for ISR to complete its RX business.*/
} flexcan_state_t;

/*! @brief FlexCAN data info from user*/
typedef struct FlexCANDataInfo
{
    flexcan_frame_format_t msg_id_type; /*!< Type of message ID (standard or extended)*/
    uint32_t data_length;               /*!< Length of Data in Bytes*/
} flexcan_data_info_t;

/*! @brief FlexCAN operation modes*/
typedef enum _flexcan_operation_modes
{
    kFlexCanNormalMode,     /*!< Normal mode or user mode*/
    kFlexCanListenOnlyMode, /*!< Listen-only mode*/
    kFlexCanLoopBackMode,   /*!< Loop-back mode*/
    kFlexCanFreezeMode,     /*!< Freeze mode*/
    kFlexCanDisableMode,    /*!< Module disable mode*/
} flexcan_operation_modes_t;

/*!
 * @brief flexCAN common information structure
 */
typedef struct _flexcan_transfer_info
{
    flexcan_state_t state;       //!< state
    flexcan_data_info_t rx_info; //!< tx info
    flexcan_data_info_t tx_info; //!< tx info
    uint32_t rxId;               //!< rx id
    uint32_t txId;               //!< tx id
    uint32_t rx_mailbox_num;     //!< rx mb number
    uint32_t tx_mailbox_num;     //!< tx mb number
    uint8_t rx_buf[512];          //!< rx buffer
    volatile uint32_t rx_buf_write_index;  //!< rx buffer write index
    uint32_t  rx_buf_read_index;   //!< rx buffer read index
} flexcan_transfer_info_t;






/*!
 * @brief hardware initialization
 */
void init_hardware(void);





//---------------------------TEMP --------------------------------------
uint8_t getBuffCnt(void);
uint8_t getBuffOffset(uint8_t offset);
//----------------------------------------------------------------------

//! @brief sending host bytes command process
void write_bytes_to_host(uint8_t * src, uint32_t lenght);


//! @brief receiving host get bytes command process
uint32_t get_bytes_received_from_host(void);

//! @brief receiving host start command process
void host_start_command_rx(uint8_t *dest);

//! @brief receiving host stop command process
void host_stop_command_rx(void);



//! @brief spi config speed process
void configure_spi_speed(uint32_t speedkhz);

//! @brief spi config settings process
void configure_spi_settings(lpspi_clock_polarity_t polarity,
							lpspi_clock_phase_t phase,
							lpspi_shift_direction_t direction);

//! @brief spi send data process
void send_spi_data(uint8_t *src, uint32_t writeLength);

//! @brief spi receiving data process
void receive_spi_data(uint8_t *dest, uint32_t readLength);


//! @brief flexCAN config speed process
void configure_can_speed(uint32_t speed);

//! @brief flexCAN config txid
void configure_can_txid(uint32_t txid);

//! @brief flexCAN config rxid
void configure_can_rxid(uint32_t rxid);

//! @brief flexCAN sending data process
void send_can_data(uint8_t *src, uint32_t writeLength);

//! @brief flexCAN receiving data process
void receive_can_data(uint8_t data, uint32_t instance);

//! @brief flexCAN read data process
void read_can_data(uint8_t *dest, uint32_t readLength);

//! @brief flexCAN reset buffer process
void reset_can_buffer(void);




//! @brief i2c config address process
void configure_i2c_address(uint8_t address);

//! @brief i2c config speed process
void configure_i2c_speed(uint32_t speedkhz);

//! @brief i2c sending data process
status_t send_i2c_data(uint8_t *src, uint32_t writeLength);

//! @brief i2c receiving data process
status_t receive_i2c_data(uint8_t *dest, uint32_t readLength);


#endif /* BUS_PAL_HARDWARE_H_ */
