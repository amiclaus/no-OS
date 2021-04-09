#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include "error.h"
#include "iio.h"
#include "iio_ad7746.h"
#include "util.h"
#include "ad7746.h"

int32_t ad7746_read_register2(struct ad7124_dev *dev, uint32_t reg,
			      uint32_t *readval)
{
	return ad7746_reg_read(dev, reg, readval, 1);
}

int32_t ad7746_write_register2(struct ad7124_dev *dev, uint32_t reg,
			      uint32_t writeval)
{
	return ad7746_reg_read(dev, reg, writeval, 1);
}

struct iio_device iio_ad7746_device = {
	// .num_ch = ARRAY_SIZE(ad7124_channels),
	// .channels = ad7124_channels,
	// .attributes = NULL,
	// .debug_attributes = NULL,
	// .buffer_attributes = NULL,
	// .prepare_transfer = iio_ad7124_update_active_channels,
	// .end_transfer = iio_ad7124_close_channels,
	// .read_dev = (int32_t (*)())iio_ad7124_read_samples,
	.debug_reg_read = (int32_t (*)())ad7746_read_register2,
	.debug_reg_write = (int32_t (*)())ad7746_write_register2
};