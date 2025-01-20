/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MICROSECONDS_H_
#define MICROSECONDS_H_

#include <stdint.h>

////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

/********************************************************************/
//! @brief Initialize timer facilities.
void microseconds_init(void);

//! @brief Shutdown the microsecond timer
void microseconds_shutdown(void);

//! @brief Read back the running tick count
uint64_t microseconds_get_ticks(void);

//! @brief Returns the conversion of ticks to actual microseconds
//!        This is used to seperate any calculations from getting a tick
//         value for speed critical scenarios
uint32_t microseconds_convert_to_microseconds(uint32_t ticks);

//! @brief Returns the conversion of microseconds to ticks
uint64_t microseconds_convert_to_ticks(uint32_t microseconds);

//! @brief Delay specified time
void microseconds_delay(uint32_t us);

//! @brief Gets the clock value used for microseconds driver
uint32_t microseconds_get_clock(void);

#endif /* MICROSECONDS_H_ */
