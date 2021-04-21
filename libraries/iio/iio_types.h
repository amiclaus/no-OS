/***************************************************************************//**
 *   @file   iio_types.h
 *   @brief  Header file for iio_types
 *   @author Cristian Pop (cristian.pop@analog.com)
********************************************************************************
 * Copyright 2013(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  - Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *  - The use of this software may or may not infringe the patent rights
 *    of one or more patent holders.  This license does not release you
 *    from the requirement that you obtain separate licenses from these
 *    patent holders to use this software.
 *  - Use of the software either in source or binary form, must be run
 *    on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/
#ifndef IIO_TYPES_H_
#define IIO_TYPES_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "tinyiiod.h"

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/


enum iio_val {
	IIO_VAL_INT = 1,
	IIO_VAL_INT_PLUS_MICRO,
	IIO_VAL_INT_PLUS_NANO,
	IIO_VAL_INT_PLUS_MICRO_DB,
	IIO_VAL_INT_MULTIPLE,
	IIO_VAL_FRACTIONAL = 10,
	IIO_VAL_FRACTIONAL_LOG2,
	IIO_VAL_CHAR
};

/**
 * @struct iio_chan_type
 * @brief IIO channel types
 */
enum iio_chan_type {
	IIO_VOLTAGE,
	IIO_CURRENT,
	IIO_ALTVOLTAGE,
	IIO_ANGL_VEL,
	IIO_TEMP,
	IIO_CAPACITANCE
};

/**
 * @struct iio_modifier
 * @brief IIO channel modifier
 */
enum iio_modifier {
	IIO_NO_MOD,
	IIO_MOD_X,
	IIO_MOD_Y,
	IIO_MOD_Z,
};

/**
 * @struct iio_ch_info
 * @brief Structure holding channel attributess.
 */
struct iio_ch_info {
	/** Channel number. TODO: refactor out the ch_ prefix. */
	int16_t ch_num;
	/** Channel direction: input/output. TODO: refactor out the ch_ prefix. */
	bool ch_out;
	/** Channel type */
	enum iio_chan_type type;
	/** Differential channel indicator */
	bool differential;
};

#define END_ATTRIBUTES_ARRAY {.name = NULL}

enum iio_attribute_shared {
	IIO_SEPARATE,
	IIO_SHARED_BY_TYPE,
	IIO_SHARED_BY_DIR,
	IIO_SHARED_BY_ALL,
};

/**
 * @struct iio_attribute
 * @brief Structure holding pointers to show and store functions.
 */
struct iio_attribute {
	/** Attribute name */
	const char *name;
	/** Attribute id */
	intptr_t priv;
	/** Whether this attribute is shared by all channels of the same type, or direction 
	 * or simply by all channels. If left uninitialized, the sharedness defaults to
	 * separate.
	*/
	enum iio_attribute_shared shared;
	/** Show function pointer */
	ssize_t (*show)(void *device, char *buf, size_t len,
			const struct iio_ch_info *channel, intptr_t priv);
	/** Store function pointer */
	ssize_t (*store)(void *device, char *buf, size_t len,
			 const struct iio_ch_info *channel, intptr_t priv);
};

/**
 * @struct iio_channel
 * @brief Struct describing the scan type
 */
struct scan_type {
	/** 's' or 'u' to specify signed or unsigned */
	char			sign;
	/** Number of valid bits of data */
	uint8_t 		realbits;
	/** Realbits + padding */
	uint8_t			storagebits;
	/** Shift right by this before masking out realbits. */
	uint8_t			shift;
	/** True if big endian, false if little endian */
	bool			is_big_endian;
};

/**
 * @struct iio_channel
 * @brief Structure holding attributes of a channel.
 */
struct iio_channel {
	/** Channel name */
	const char		*name;
	/** May be used to label channel attributes with an informative name. */
	const char 		*extend_name;
	/** Chanel type */
	enum iio_chan_type	ch_type;
	/** Channel number when the same channel type */
	int 			channel;
	/** If modified is set, this provides the modifier. E.g. IIO_MOD_X
	 *  for angular rate when applied to channel2 will make make the
	 *  IIO_ANGL_VEL have anglvel_x which corresponds to the x-axis. */
	int 			channel2;
	/** Driver specific identifier. */
	unsigned long		address;
	/** Index to give ordering in scans when read  from a buffer. */
	int			scan_index;
	/** */
	struct scan_type	*scan_type;
	/** Array of attributes. Last one should have its name set to NULL */
	struct iio_attribute	*attributes;
	/** if true, the channel is an output channel */
	bool			ch_out;
	/** Set if channel has a modifier. Use channel2 property to
	 *  select the modifier to use.*/
	bool			modified;
	/** Specify if channel has a numerical index. If not set, channel
	 *  number will be suppressed. */
	bool			indexed;
	/* Set if the channel is differential. */
	bool			diferential;
};

struct iio_data_buffer {
	uint32_t	size;
	void		*buff;
};

/**
 * @struct iio_device
 * @brief Structure holding channels and attributes of a device.
 */
struct iio_device {
	/** Device number of channels */
	uint16_t num_ch;
	/** List of channels */
	struct iio_channel *channels;
	/** Array of attributes. Last one should have its name set to NULL */
	struct iio_attribute *attributes;
	/** Array of attributes. Last one should have its name set to NULL */
	struct iio_attribute *debug_attributes;
	/** Array of attributes. Last one should have its name set to NULL */
	struct iio_attribute *buffer_attributes;
	/** Called before a transfer starts. The device should activate the
	 * channels from the mask */
	int32_t (*prepare_transfer)(void *dev, uint32_t mask);
	/** Called after a tranfer ends */
	int32_t (*end_transfer)(void *dev);
	/* Numbers of bytes will be:
	 * samples * (storage_size_of_first_active_ch / 8) * nb_active_channels
	 */
	int32_t	(*read_dev)(void *dev, void *buff, uint32_t nb_samples);
	/* Numbers of bytes will be:
	 * samples * (storage_size_of_first_active_ch / 8) * nb_active_channels
	 */
	int32_t	(*write_dev)(void *dev, void *buff, uint32_t nb_samples);
	/* Read device register */
	int32_t (*debug_reg_read)(void *dev, uint32_t reg, uint32_t *readval);
	/* Write device register */
	int32_t (*debug_reg_write)(void *dev, uint32_t reg, uint32_t writeval);

};

#endif /* IIO_TYPES_H_ */
