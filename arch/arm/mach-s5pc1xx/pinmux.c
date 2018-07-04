// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2012 Samsung Electronics.
 * Abhilash Kesavan <a.kesavan@samsung.com>
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/gpio.h>
#include <asm/arch/pinmux.h>

static int s5pc110_mmc_config(int peripheral, int flags)
{
	int i, start = 0, start_ext = 0;
	unsigned int func, ext_func;

	switch (peripheral) {
	case PERIPH_ID_SDMMC0:
		start = S5PC110_GPIO_G00;
		start_ext = S5PC110_GPIO_G13;
		func = S5P_GPIO_FUNC(0x2);
		ext_func = S5P_GPIO_FUNC(0x3);
		break;
	case PERIPH_ID_SDMMC2:
		start = S5PC110_GPIO_G20;
		start_ext = S5PC110_GPIO_G33;
		func = S5P_GPIO_FUNC(0x2);
		ext_func = S5P_GPIO_FUNC(0x3);
		break;
	default:
		return -1;
	}
	for (i = start; i < (start + 7); i++) {
		if (i == (start + 2))
			continue;
		gpio_cfg_pin(i,  func);
		gpio_set_pull(i, S5P_GPIO_PULL_NONE);
		gpio_set_drv(i, S5P_GPIO_DRV_4X);
	}

	if (flags & PINMUX_FLAG_8BIT_MODE) {
		for (i = start_ext; i < (start_ext + 4); i++) {
			gpio_cfg_pin(i,  ext_func);
			gpio_set_pull(i, S5P_GPIO_PULL_NONE);
			gpio_set_drv(i, S5P_GPIO_DRV_4X);
		}
	}

	return 0;
}

static void s5pc110_uart_config(int peripheral)
{
	int i, start, count;

	switch (peripheral) {
	case PERIPH_ID_UART0:
		start = S5PC110_GPIO_A00;
		count = 4;
		break;
	case PERIPH_ID_UART1:
		start = S5PC110_GPIO_A04;
		count = 4;
		break;
	case PERIPH_ID_UART2:
		start = S5PC110_GPIO_A10;
		count = 2;
		break;
	case PERIPH_ID_UART3:
		start = S5PC110_GPIO_A12;
		count = 2;
		break;
	default:
		debug("%s: invalid peripheral %d", __func__, peripheral);
		return;
	}
	for (i = start; i < (start + count); i++) {
		gpio_set_pull(i, S5P_GPIO_PULL_NONE);
		gpio_cfg_pin(i, S5P_GPIO_FUNC(0x2));
	}
}

static int s5pc110_pinmux_config(int peripheral, int flags)
{
	switch (peripheral) {
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
	case PERIPH_ID_UART3:
		s5pc110_uart_config(peripheral);
		break;
	case PERIPH_ID_SDMMC0:
	case PERIPH_ID_SDMMC2:
		return s5pc110_mmc_config(peripheral, flags);
	case PERIPH_ID_SDMMC1:
	case PERIPH_ID_SDMMC3:
		debug("SDMMC device %d not implemented\n", peripheral);
		return -1;
	default:
		debug("%s: invalid peripheral %d", __func__, peripheral);
		return -1;
	}

	return 0;
}

int exynos_pinmux_config(int peripheral, int flags)
{
	if (cpu_is_s5pc110())
		return s5pc110_pinmux_config(peripheral, flags);

	debug("pinmux functionality not supported\n");

	return -1;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int s5pc110_pinmux_decode_periph_id(const void *blob, int node)
{
	int err;
	u32 cell[1];

	err = fdtdec_get_int_array(blob, node, "periph-id", cell,
					ARRAY_SIZE(cell));
	if (err) {
		debug(" invalid peripheral id\n");
		return PERIPH_ID_NONE;
	}

	return cell[0];
}

int pinmux_decode_periph_id(const void *blob, int node)
{
	if (cpu_is_s5pc110())
		return  s5pc110_pinmux_decode_periph_id(blob, node);

	return PERIPH_ID_NONE;
}
#endif
