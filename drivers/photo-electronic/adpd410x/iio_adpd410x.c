/***************************************************************************//**
 *   @file   iio_adpd410x.c
 *   @brief  Implementation of adpd410x iio.
 *   @author Antoniu Miclaus (antoniu.miclaus@analog.com)
********************************************************************************
 * Copyright 2021(c) Analog Devices, Inc.
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

#include "iio_types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "adpd410x.h"
#include "util.h"
#include "error.h"

/**
 * @brief Read ADC Channel data.
 * @param device - Device driver descriptor.
 * @param buf - Input buffer.
 * @param len - Length of the input buffer.
 * @param channel - IIO channel information.
 * @return Number of bytes printed in the output buffer, or negative error code.
 */
static ssize_t adpd410x_iio_channel_read_raw(void *device, char *buf,
		size_t len,
		const struct iio_ch_info *channel)
{
	struct adpd410x_dev *dev = (struct adpd410x_dev *)device;
	int32_t ret, data[8];

	ret = adpd410x_set_opmode(dev, ADPD410X_GOMODE);
	if (ret != SUCCESS)
		return ret;

	ret = adpd410x_get_data(dev, &data);
	if (ret != SUCCESS)
		return ret;

	ret = adpd410x_set_opmode(dev, ADPD410X_STANDBY);
	if (ret != SUCCESS)
		return ret;

	return snprintf(buf, len, "%d", data[channel->ch_num]);
}

/**
 * @brief Set device sampling frequency.
 * @param device - Device driver descriptor.
 * @param buf - Input buffer.
 * @param len - Length of the input buffer (not used in this case).
 * @param channel - IIO channel information.
 * @return Number of bytes printed in the output buffer, or negative error code.
 */
static ssize_t adpd410x_iio_set_sampling_freq(void *device, char *buf,
		size_t len,
		const struct iio_ch_info *channel, intptr_t priv)
{
	struct adpd410x_dev *dev = (struct adpd410x_dev *)device;
	int32_t ret;

	uint32_t freq = srt_to_uint32(buf);

	ret = adpd410x_set_sampling_freq(dev, freq);
	if (ret != SUCCESS)
		return ret;

	return len;
}

/**
 * @brief Set number of active time slots.
 * @param device - Device driver descriptor.
 * @param buf - Input buffer.
 * @param len - Length of the input buffer (not used in this case).
 * @param channel - IIO channel information.
 * @return Number of bytes printed in the output buffer, or negative error code.
 */
static ssize_t adpd410x_iio_set_last_timeslot(void *device, char *buf,
		size_t len,
		const struct iio_ch_info *channel, intptr_t priv)
{
	struct adpd410x_dev *dev = (struct adpd410x_dev *)device;
	int32_t ret;

	enum adpd410x_timeslots timeslot_no = srt_to_uint32(buf);

	ret = adpd410x_set_last_timeslot(dev, timeslot_no);
	if (ret != SUCCESS)
		return ret;

	return len;
}

/**
 * @brief Set device sampling frequency.
 * @param device - Device driver descriptor.
 * @param buf - Input buffer.
 * @param len - Length of the input buffer (not used in this case).
 * @param channel - IIO channel information.
 * @return Number of bytes printed in the output buffer, or negative error code.
 */
static ssize_t adpd410x_iio_set_opmode(void *device, char *buf, size_t len,
				       const struct iio_ch_info *channel, intptr_t priv)
{
	struct adpd410x_dev *dev = (struct adpd410x_dev *)device;
	int32_t ret;

	enum adpd410x_opmode mode = srt_to_uint32(buf);

	ret = adpd410x_set_opmode(dev, mode);
	if (ret != SUCCESS)
		return ret;

	return len;
}

/** IIO channel attributes*/
static struct iio_attribute adpd410x_iio_channel_attributes[] = {
	{
		.name = "raw",
		.show = adpd410x_iio_channel_read_raw,
		.store = NULL
	},
	END_ATTRIBUTES_ARRAY,
};

/** Channel Scan Type */
static struct scan_type channel_scan_type = {
	.sign = 'u',
	.realbits = 16,
	.storagebits = 32,
	.shift = 0,
	.is_big_endian = false
};

#define ADPD410X_IIO_CHANN_DEF(nm, ch) \
	{ \
		.name = nm, \
		.ch_type = IIO_VOLTAGE, \
		.channel = ch, \
		.scan_type = &channel_scan_type, \
		.attributes = adpd410x_iio_channel_attributes, \
		.ch_out = false, \
		.indexed = 1, \
		.diferential = false, \
	}

/** IIO Channels */
static struct iio_channel adpd410x_iio_channels[] = {
	ADPD410X_IIO_CHANN_DEF("channel0", 0),
	ADPD410X_IIO_CHANN_DEF("channel1", 1),
	ADPD410X_IIO_CHANN_DEF("channel2", 2),
	ADPD410X_IIO_CHANN_DEF("channel3", 3),
	ADPD410X_IIO_CHANN_DEF("channel4", 4),
	ADPD410X_IIO_CHANN_DEF("channel5", 5),
	ADPD410X_IIO_CHANN_DEF("channel6", 6),
	ADPD410X_IIO_CHANN_DEF("channel7", 7),
	END_ATTRIBUTES_ARRAY
};

/** IIO attributes */
static struct iio_attribute adpd410x_iio_attributes[] = {
	{
		.name = "sampling_frequency",
		.show = NULL,
		.store = adpd410x_iio_set_sampling_freq,
	},
	{
		.name = "last_timeslot",
		.show = NULL,
		.store = adpd410x_iio_set_last_timeslot,
	},
	{
		.name = "operation_mode",
		.show = NULL,
		.store = adpd410x_iio_set_opmode,
	},
	END_ATTRIBUTES_ARRAY,
};

/** IIO Descriptor */
struct iio_device const adpd410x_iio_descriptor = {
	.num_ch = 8,
	.channels = adpd410x_iio_channels,
	.attributes = adpd410x_iio_attributes,
	.debug_reg_read = (int32_t (*)())adpd410x_reg_read,
	.debug_reg_write = (int32_t (*)())adpd410x_reg_write,
};

