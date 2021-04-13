#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include "error.h"
#include "iio.h"
#include "iio_ad7746.h"
#include "util.h"
#include "ad7746.h"

int32_t ad7746_read_register2(struct ad7746_dev *dev, uint32_t reg,
			      uint32_t *readval)
{
	return ad7746_reg_read(dev, reg, readval, 1);
}

int32_t ad7746_write_register2(struct ad7746_dev *dev, uint32_t reg,
			      uint32_t writeval)
{
	return ad7746_reg_read(dev, reg, writeval, 1);
}

static struct iio_attribute ad7746_iio_vin_attrs[] = {
	{
		.name = "raw",
		.show = NULL,
		.store = NULL
	},
	{
		.name = "scale",
		.show = NULL,
		.store = NULL
	},
	{
		.name = "samp_freq",
		.show = NULL,
		.store = NULL
	},
	END_ATTRIBUTES_ARRAY
};

static struct iio_attribute ad7746_iio_cin_attrs[] = {
	{
		.name = "raw",
		.show = NULL,
		.store = NULL
	},
	{
		.name = "scale",
		.show = NULL,
		.store = NULL
	},
	{
		.name = "offset",
		.show = NULL,
		.store = NULL
	},
	{
		.name = "samp_freq",
		.show = NULL,
		.store = NULL
	},
	{
		.name = "calibscale",
		.show = NULL,
		.store = NULL
	},
	{
		.name = "calibbias",
		.show = NULL,
		.store = NULL
	},
	END_ATTRIBUTES_ARRAY
};

static struct iio_attribute ad7746_iio_temp_attrs[] = {
	{
		.name = "processed",
		.show = NULL,
		.store = NULL
	},
	END_ATTRIBUTES_ARRAY
};

enum ad7746_chan {
	VIN,
	VIN_VDD,
	TEMP_INT,
	TEMP_EXT,
	CIN1,
	CIN1_DIFF,
	CIN2,
	CIN2_DIFF,
};

static struct iio_channel ad7746_channels[] = {
	[VIN] = {
		.ch_type = IIO_VOLTAGE,
		.indexed = 1,
		.channel = 0,
		.attributes = ad7746_iio_vin_attrs,
		//.address = AD7746_REG_VT_DATA_HIGH << 8 | AD7746_VIN_EXT_VIN,
		.ch_out = false,
	},
	[VIN_VDD] = {
		.ch_type = IIO_VOLTAGE,
		.indexed = 1,
		.channel = 1,
		//.extend_name = "supply",
		.attributes = ad7746_iio_vin_attrs,
		//.address = AD7746_REG_VT_DATA_HIGH << 8 | AD7746_VTMD_VDD_MON,
		.ch_out = false,
	},
	[TEMP_INT] = {
		.ch_type = IIO_TEMP,
		.indexed = 1,
		.channel = 0,
		.attributes = ad7746_iio_temp_attrs,
		//.address = AD7746_REG_VT_DATA_HIGH << 8 | AD7746_VTMD_INT_TEMP,
		.ch_out = false,
	},
	[TEMP_EXT] = {
		.ch_type = IIO_TEMP,
		.indexed = 1,
		.channel = 1,
		.attributes = ad7746_iio_temp_attrs,
		//.address = AD7746_REG_VT_DATA_HIGH << 8 | AD7746_VTMD_EXT_TEMP,
		.ch_out = false,
	},
	[CIN1] = {
		.ch_type = IIO_CAPACITANCE,
		.indexed = 1,
		.channel = 0,
		.attributes = ad7746_iio_cin_attrs,
		//.address = AD7746_REG_CAP_DATA_HIGH << 8,
		.ch_out = false,
	},
	[CIN1_DIFF] = {
		.ch_type = IIO_CAPACITANCE,
		.diferential = 1,
		.indexed = 1,
		.channel = 0,
		.channel2 = 2,
		.attributes = ad7746_iio_cin_attrs,
		// .address = AD7746_REG_CAP_DATA_HIGH << 8 | AD7746_CAPSETUP_CAPDIFF_MSK,
		.ch_out = false,
	},
	[CIN2] = {
		.ch_type = IIO_CAPACITANCE,
		.indexed = 1,
		.channel = 1,
		.attributes = ad7746_iio_cin_attrs,
		//.address = AD7746_REG_CAP_DATA_HIGH << 8 | AD7746_CAPSETUP_CIN2_MSK,
		.ch_out = false,
	},
	[CIN2_DIFF] = {
		.ch_type = IIO_CAPACITANCE,
		.diferential = 1,
		.indexed = 1,
		.channel = 1,
		.channel2 = 3,
		.attributes = ad7746_iio_cin_attrs,
		//.address = AD7746_REG_CAP_DATA_HIGH << 8 | AD7746_CAPSETUP_CAPDIFF_MSK | AD7746_CAPSETUP_CIN2_MSK,
		.ch_out = false,
	}
};

struct iio_device iio_ad7746_device = {
	.num_ch = ARRAY_SIZE(ad7746_channels),
	.channels = ad7746_channels,
	// .attributes = NULL,
	// .debug_attributes = NULL,
	// .buffer_attributes = NULL,
	// .prepare_transfer = iio_ad7124_update_active_channels,
	// .end_transfer = iio_ad7124_close_channels,
	// .read_dev = (int32_t (*)())iio_ad7124_read_samples,
	.debug_reg_read = (int32_t (*)())ad7746_read_register2,
	.debug_reg_write = (int32_t (*)())ad7746_write_register2
};