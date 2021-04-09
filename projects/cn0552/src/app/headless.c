/***************************************************************************//**
* @file headless.c
* @brief CN0552 IIO demo project main file.
* @author Darius Berghe (darius.berghe@analog.com)
********************************************************************************
* Copyright 2021(c) Analog Devices, Inc.
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* - Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* - Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in
* the documentation and/or other materials provided with the
* distribution.
* - Neither the name of Analog Devices, Inc. nor the names of its
* contributors may be used to endorse or promote products derived
* from this software without specific prior written permission.
* - The use of this software may or may not infringe the patent rights
* of one or more patent holders. This license does not release you
* from the requirement that you obtain separate licenses from these
* patent holders to use this software.
* - Use of the software either in source or binary form, must be run
* on or directly connected to an Analog Devices Inc. component.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/platform.h>
#include "adi_initialize.h"
#include <drivers/pwr/adi_pwr.h>

#include "iio.h"
#include "iio_ad7746.h"
#include "iio_app.h"
#include "ad7746.h"
#include "irq.h"
#include "irq_extra.h"
#include "i2c.h"
#include "delay.h"
#include "print_log.h"
#include "uart.h"
#include "uart_extra.h"
#include "platform_init.h"
#include "parameters.h"

int main(void)
{
	int32_t ret;
	struct ad7746_init_param adcip;
	struct ad7746_dev *adc = NULL;

	ret = platform_init();
	if (ret < 0)
		goto error;

	adcip.i2c_init = (struct i2c_init_param) {
		.max_speed_hz = I2C_SPEED,
		.slave_address = AD7746_ADDRESS,
	};
	adcip.setup = (struct ad7746_setup) {
		.cap = {
			.capen = true,
			.cin2 = true,
			.capdiff = false,
			.capchop = true
		},
		.vt = {
			.vten = true,
			.vtmd = AD7746_VTMD_INT_TEMP,
			.extref = false,
			.vtshort = false,
			.vtchop = true
		},
		.exc = {
			.clkctrl = false,
			.excon = false,
			.excb = AD7746_EXC_PIN_DISABLED,
			.exca = AD7746_EXC_PIN_NORMAL,
			.exclvl = AD7746_EXCLVL_4_DIV_8
		},
		.config = {
			.vtf = 0,
			.capf = 0,
			.md = AD7746_MODE_CONT
		}
	};

	pr_info("Hello!\n");

	ret = ad7746_init(&adc, &adcip);
	if (ret < 0) {
		pr_err("AD7746 initialization failed with: %d\n", ret);
		goto error;
	}

	struct iio_app_device devices[] = {
		IIO_APP_DEVICE("ad7746", adc, &iio_ad7746_device,
			       NULL, NULL)
	};

	return iio_app_run(devices, 1);
error:
	pr_info("Bye!\n");
	return 0;
}
