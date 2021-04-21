#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include "error.h"
#include "delay.h"
#include "iio.h"
#include "iio_ad7746.h"
#include "util.h"
#include "ad7746.h"

static int32_t _ad7746_read_register2(struct ad7746_dev *dev, uint32_t reg,
			      uint32_t *readval)
{
	return ad7746_reg_read(dev, reg, readval, 1);
}

static int32_t _ad7746_write_register2(struct ad7746_dev *dev, uint32_t reg,
			      uint32_t writeval)
{
	return ad7746_reg_read(dev, reg, writeval, 1);
}

static inline bool _capdiff(struct ad7746_cap *cap1, struct ad7746_cap *cap2)
{
	return cap1->capchop != cap2->capchop ||
		cap1->capdiff != cap2->capdiff ||
		cap1->capen != cap2->capen ||
		cap1->cin2 != cap2->cin2;
}

static inline bool _vtdiff(struct ad7746_vt *vt1, struct ad7746_vt *vt2)
{
	return vt1->extref != vt2->extref ||
		vt1->vtchop != vt2->vtchop ||
		vt1->vten != vt2->vten ||
		vt1->vtmd != vt2->vtmd ||
		vt1->vtshort != vt2->vtshort;
}

static inline bool _configdiff(struct ad7746_config *c1, struct ad7746_config *c2)
{
	return c1->md != c2->md ||
		c1->capf != c2->capf ||
		c1->vtf != c2->vtf;
}

/* Values are Update Rate (Hz), Conversion Time (ms) + 1*/
static const unsigned char ad7746_vt_filter_rate_table[][2] = {
	{50, 20 + 1}, {31, 32 + 1}, {16, 62 + 1}, {8, 122 + 1},
};

static const unsigned char ad7746_cap_filter_rate_table[][2] = {
	{91, 11 + 1}, {84, 12 + 1}, {50, 20 + 1}, {26, 38 + 1},
	{16, 62 + 1}, {13, 77 + 1}, {11, 92 + 1}, {9, 110 + 1},
};

// perform channel selection
static int ad7746_select_channel(void *device, struct iio_ch_info *ch_info)
{
	struct ad7746_dev *desc = (struct ad7746_dev *)device;
	struct ad7746_cap cap = desc->setup.cap;
	struct ad7746_vt vt = desc->setup.vt;
	int32_t ret, delay, idx;

	switch (ch_info->type) {
	case IIO_CAPACITANCE:
		cap.capen = true;
		vt.vten = false;
		idx = desc->setup.config.capf;
		delay = ad7746_cap_filter_rate_table[idx][1];

		if (desc->capdac_set != ch_info->ch_num) {
			ret = ad7746_set_cap_dac_a(desc, true, desc->capdac[ch_info->ch_num][0]);
			if (ret < 0)
				return ret;
			ret = ad7746_set_cap_dac_b(desc, true, desc->capdac[ch_info->ch_num][1]);
			if (ret < 0)
				return ret;

			desc->capdac_set = ch_info->ch_num;
		}
		break;
	case IIO_VOLTAGE:
	case IIO_TEMP:
		vt.vten = true;
		cap.capen = false;
		idx = desc->setup.config.vtf;
		delay = ad7746_cap_filter_rate_table[idx][1];
		break;
	default:
		return -EINVAL;
	}

	if (_capdiff(&desc->setup.cap, &cap)) {
		ret = ad7746_set_cap(desc, cap);
		if (ret < 0)
			return ret;
	}
	
	if (_vtdiff(&desc->setup.vt, &vt)) {
		ret = ad7746_set_vt(desc, vt);
		if (ret < 0)
			return ret;
	}

	return delay;
}

static ssize_t ad7746_iio_read_raw(void *device, char *buf, size_t len,
					const struct iio_ch_info *channel, intptr_t priv)
{
	struct ad7746_dev *desc = (struct ad7746_dev *)device;
	int32_t ret, delay, value;
	uint32_t reg;
	struct ad7746_config c;

	ret = ad7746_select_channel(desc, channel);
	if (ret < 0)
		return ret;
	delay = ret;

	c = desc->setup.config;
	c.md = AD7746_MODE_SINGLE;

	if(_configdiff(&desc->setup.config, &c))
	{
		ret = ad7746_set_config(desc, c);
		if (ret < 0)
			return ret;
	}

	mdelay(delay);

	switch (channel->type) {
	case IIO_TEMP:
		ret = ad7746_get_vt_data(desc, &reg);
		if (ret < 0)
			return ret;

		value = (reg & 0xffffff) - 0x800000;
		/*
		* temperature in milli degrees Celsius
		* T = ((*val / 2048) - 4096) * 1000
		*/
		value = (value * 125) / 256;
		break;
	case IIO_VOLTAGE:
		ret = ad7746_get_vt_data(desc, &reg);
		if (ret < 0)
			return ret;

		value = (reg & 0xffffff) - 0x800000;

		if (channel->ch_num == 1) /* supply_raw */
			value = value * 6;
		break;
	case IIO_CAPACITANCE:
		ret = ad7746_get_cap_data(desc, &reg);
		if (ret < 0)
			return ret;

		value = (reg & 0xffffff) - 0x800000;
		break;
	default:
		return -EINVAL;
	}

	return iio_format_value(buf, len, IIO_VAL_INT, 1, &value);
}

static ssize_t ad7746_iio_read_scale(void *device, char *buf, size_t len,
					const struct iio_ch_info *channel, intptr_t priv)
{
	struct ad7746_dev *desc = (struct ad7746_dev *)device;
	int32_t valt;
	int32_t vals[2];

	switch (channel->type) {
	case IIO_CAPACITANCE:
		/* 8.192pf / 2^24 */
		vals[0] =  0;
		vals[1] = 488;
		valt = IIO_VAL_INT_PLUS_NANO;
		break;
	case IIO_VOLTAGE:
		/* 1170mV / 2^23 */
		vals[0] = 1170;
		vals[1] = 23;
		valt = IIO_VAL_FRACTIONAL_LOG2;
		break;
	default:
		return -EINVAL;
		break;
	}

	return iio_format_value(buf, len, valt, 2, vals);
}

static ssize_t ad7746_iio_read_offset(void *device, char *buf, size_t len,
					const struct iio_ch_info *channel, intptr_t priv)
{
	struct ad7746_dev *desc = (struct ad7746_dev *)device;
	int32_t value;

	value = (desc->capdac[channel->ch_num][channel->differential] & AD7746_CAPDAC_DACP_MSK) * 338646;

	return iio_format_value(buf, len, IIO_VAL_INT, 1, &value);
}

static ssize_t ad7746_iio_read_samp_freq(void *device, char *buf, size_t len,
					const struct iio_ch_info *channel, intptr_t priv)
{
	struct ad7746_dev *desc = (struct ad7746_dev *)device;
	int32_t value;

	switch (channel->type) {
	case IIO_CAPACITANCE:
		value = ad7746_cap_filter_rate_table[desc->setup.config.capf][0];
		break;
	case IIO_VOLTAGE:
		value = ad7746_cap_filter_rate_table[desc->setup.config.vtf][0];
		break;
	default:
		return -EINVAL;
		break;
	}

	return iio_format_value(buf, len, IIO_VAL_INT, 1, &value);
}

static struct iio_attribute ad7746_iio_vin_attrs[] = {
	{
		.name = "raw",
		.show = ad7746_iio_read_raw,
		.store = NULL
	},
	{
		.name = "scale",
		.shared = IIO_SHARED_BY_TYPE,
		.show = ad7746_iio_read_scale,
		.store = NULL
	},
	{
		.name = "sampling_frequency",
		.shared = IIO_SHARED_BY_TYPE,
		.show = ad7746_iio_read_samp_freq,
		.store = NULL
	},
	END_ATTRIBUTES_ARRAY
};

static struct iio_attribute ad7746_iio_cin_attrs[] = {
	{
		.name = "raw",
		.show = ad7746_iio_read_raw,
		.store = NULL
	},
	{
		.name = "scale",
		.shared = IIO_SHARED_BY_TYPE,
		.show = ad7746_iio_read_scale,
		.store = NULL
	},
	{
		.name = "offset",
		.show = ad7746_iio_read_offset,
		.store = NULL
	},
	{
		.name = "sampling_frequency",
		.shared = IIO_SHARED_BY_TYPE,
		.show = ad7746_iio_read_samp_freq,
		.store = NULL
	},
	{
		.name = "calibscale",
		.show = NULL,
		.store = NULL
	},
	{
		.name = "calibbias",
		.shared = IIO_SHARED_BY_TYPE,
		.show = NULL,
		.store = NULL
	},
	END_ATTRIBUTES_ARRAY
};

static struct iio_attribute ad7746_iio_temp_attrs[] = {
	{
		.name = "input",
		.show = ad7746_iio_read_raw,
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
		.address = AD7746_VIN_EXT_VIN,
		.ch_out = false,
	},
	[VIN_VDD] = {
		.ch_type = IIO_VOLTAGE,
		.indexed = 1,
		.channel = 1,
		.extend_name = "supply",
		.attributes = ad7746_iio_vin_attrs,
		.address = AD7746_VTMD_VDD_MON,
		.ch_out = false,
	},
	[TEMP_INT] = {
		.ch_type = IIO_TEMP,
		.indexed = 1,
		.channel = 0,
		.attributes = ad7746_iio_temp_attrs,
		.address = AD7746_VTMD_INT_TEMP,
		.ch_out = false,
	},
	[TEMP_EXT] = {
		.ch_type = IIO_TEMP,
		.indexed = 1,
		.channel = 1,
		.attributes = ad7746_iio_temp_attrs,
		.address = AD7746_VTMD_EXT_TEMP,
		.ch_out = false,
	},
	[CIN1] = {
		.ch_type = IIO_CAPACITANCE,
		.indexed = 1,
		.channel = 0,
		.attributes = ad7746_iio_cin_attrs,
		.ch_out = false,
	},
	[CIN1_DIFF] = {
		.ch_type = IIO_CAPACITANCE,
		.diferential = 1,
		.indexed = 1,
		.channel = 0,
		.channel2 = 2,
		.attributes = ad7746_iio_cin_attrs,
		 .address = AD7746_CAPSETUP_CAPDIFF_MSK,
		.ch_out = false,
	},
	[CIN2] = {
		.ch_type = IIO_CAPACITANCE,
		.indexed = 1,
		.channel = 1,
		.attributes = ad7746_iio_cin_attrs,
		.address = AD7746_CAPSETUP_CIN2_MSK,
		.ch_out = false,
	},
	[CIN2_DIFF] = {
		.ch_type = IIO_CAPACITANCE,
		.diferential = 1,
		.indexed = 1,
		.channel = 1,
		.channel2 = 3,
		.attributes = ad7746_iio_cin_attrs,
		.address = AD7746_CAPSETUP_CAPDIFF_MSK | AD7746_CAPSETUP_CIN2_MSK,
		.ch_out = false,
	}
};

struct iio_device iio_ad7746_device = {
	.num_ch = ARRAY_SIZE(ad7746_channels),
	.channels = ad7746_channels,
	.attributes = NULL,
	.debug_attributes = NULL,
	.buffer_attributes = NULL,
	.prepare_transfer = NULL,
	.end_transfer = NULL,
	.read_dev = NULL,
	.debug_reg_read = (int32_t (*)())_ad7746_read_register2,
	.debug_reg_write = (int32_t (*)())_ad7746_write_register2
};