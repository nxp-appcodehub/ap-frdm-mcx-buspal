/*
 * Copyright (c) 2014, Freescale Semiconductor, Inc.
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
#ifndef BUSPAL_CONFIG_H_
#define BUSPAL_CONFIG_H_

#include "fsl_common.h"

#define _PORT_UART_RX 				PORTC
#define _PIN_UART_RX 				2U
#define _PORT_UART_TX				PORTC
#define _PIN_UART_TX				3U
#define _DEFAULT_UART_BAUDRATE		57600
#define _BUSPAL_UART_INSTANCE		1

#define _PORT_I2C_SCL				PORTB
#define _PIN_I2C_SCL				5U
#define _PORT_I2C_SDA				PORTB
#define _PIN_I2C_SDA				4U
#define _DEFAULT_I2C_SPEED			100000
#define _BUSPAL_I2C_INSTANCE		1

#define _PORT_SPI_SDO				PORTB
#define _PIN_SPI_SDO				3U
#define _PORT_SPI_SDI				PORTB
#define _PIN_SPI_SDI				1U
#define _PORT_SPI_SCK				PORTB
#define _PIN_SPI_SCK				2U
#define _PORT_SPI_CS				PORTB
#define _PIN_SPI_CS					0U
#define _SPI_PCS					0U
#define _DEFAULT_SPI_SPEED			100000
#define _BUSPAL_SPI_INSTANCE		1

#define _PORT_CAN_TX                PORTC
#define _PIN_CAN_TX                 4U
#define _PORT_CAN_RX                PORTC
#define _PIN_CAN_RX                 5U
#define _DEFAULT_CAN_SPEED          1000000
#define _BUSPAL_CAN_INSTANCE		0

#endif /* BUSPAL_CONFIG_H_ */
