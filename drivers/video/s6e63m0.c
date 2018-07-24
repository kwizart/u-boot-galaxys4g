// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on simple_panel.c
 */

#include <common.h>
#include <dm.h>
#include <panel.h>
#include <spi.h>
#include <asm/gpio.h>
#include <power/regulator.h>

#define SLEEPMSEC		0x1000
#define ENDDEF			0x2000
#define	DEFMASK			0xFF00
#define COMMAND_ONLY		0xFE
#define DATA_ONLY		0xFF

struct s6e63m0_panel_priv {
	struct udevice *vdd3;
	struct udevice *vci;
	struct spi_slave *spi;
	struct gpio_desc reset;
};

static const unsigned short seq_panel_condition_set[] = {
	0xF8, 0x01,
	DATA_ONLY, 0x27,
	DATA_ONLY, 0x27,
	DATA_ONLY, 0x07,
	DATA_ONLY, 0x07,
	DATA_ONLY, 0x54,
	DATA_ONLY, 0x9f,
	DATA_ONLY, 0x63,
	DATA_ONLY, 0x86,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0x0d,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,

	ENDDEF, 0x0000
};

static const unsigned short seq_display_condition_set[] = {
	0xf2, 0x02,
	DATA_ONLY, 0x03,
	DATA_ONLY, 0x1c,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x10,

	0xf7, 0x03,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,

	ENDDEF, 0x0000
};

static const unsigned short seq_gamma_setting[] = {
	0xfa, 0x00,
	DATA_ONLY, 0x18,
	DATA_ONLY, 0x08,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0x64,
	DATA_ONLY, 0x56,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0xb6,
	DATA_ONLY, 0xba,
	DATA_ONLY, 0xa8,
	DATA_ONLY, 0xac,
	DATA_ONLY, 0xb1,
	DATA_ONLY, 0x9d,
	DATA_ONLY, 0xc1,
	DATA_ONLY, 0xc1,
	DATA_ONLY, 0xb7,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x9c,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x9f,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0xd6,

	0xfa, 0x01,

	ENDDEF, 0x0000
};

static const unsigned short seq_etc_condition_set[] = {
	0xf6, 0x00,
	DATA_ONLY, 0x8c,
	DATA_ONLY, 0x07,

	0xb3, 0xc,

	0xb5, 0x2c,
	DATA_ONLY, 0x12,
	DATA_ONLY, 0x0c,
	DATA_ONLY, 0x0a,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x0e,
	DATA_ONLY, 0x17,
	DATA_ONLY, 0x13,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x2a,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1b,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x17,

	DATA_ONLY, 0x2b,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x3a,
	DATA_ONLY, 0x34,
	DATA_ONLY, 0x30,
	DATA_ONLY, 0x2c,
	DATA_ONLY, 0x29,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x25,
	DATA_ONLY, 0x23,
	DATA_ONLY, 0x21,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x1e,
	DATA_ONLY, 0x1e,

	0xb6, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x11,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,

	DATA_ONLY, 0x55,
	DATA_ONLY, 0x55,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,

	0xb7, 0x2c,
	DATA_ONLY, 0x12,
	DATA_ONLY, 0x0c,
	DATA_ONLY, 0x0a,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x0e,
	DATA_ONLY, 0x17,
	DATA_ONLY, 0x13,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x2a,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1b,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x17,

	DATA_ONLY, 0x2b,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x3a,
	DATA_ONLY, 0x34,
	DATA_ONLY, 0x30,
	DATA_ONLY, 0x2c,
	DATA_ONLY, 0x29,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x25,
	DATA_ONLY, 0x23,
	DATA_ONLY, 0x21,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x1e,
	DATA_ONLY, 0x1e,

	0xb8, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x11,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,

	DATA_ONLY, 0x55,
	DATA_ONLY, 0x55,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,

	0xb9, 0x2c,
	DATA_ONLY, 0x12,
	DATA_ONLY, 0x0c,
	DATA_ONLY, 0x0a,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x0e,
	DATA_ONLY, 0x17,
	DATA_ONLY, 0x13,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x2a,
	DATA_ONLY, 0x24,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x1b,
	DATA_ONLY, 0x1a,
	DATA_ONLY, 0x17,

	DATA_ONLY, 0x2b,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x3a,
	DATA_ONLY, 0x34,
	DATA_ONLY, 0x30,
	DATA_ONLY, 0x2c,
	DATA_ONLY, 0x29,
	DATA_ONLY, 0x26,
	DATA_ONLY, 0x25,
	DATA_ONLY, 0x23,
	DATA_ONLY, 0x21,
	DATA_ONLY, 0x20,
	DATA_ONLY, 0x1e,
	DATA_ONLY, 0x1e,

	0xba, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x11,
	DATA_ONLY, 0x22,
	DATA_ONLY, 0x33,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,
	DATA_ONLY, 0x44,

	DATA_ONLY, 0x55,
	DATA_ONLY, 0x55,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,
	DATA_ONLY, 0x66,

	0xc1, 0x4d,
	DATA_ONLY, 0x96,
	DATA_ONLY, 0x1d,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x01,
	DATA_ONLY, 0xdf,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x03,
	DATA_ONLY, 0x1f,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x00,
	DATA_ONLY, 0x03,
	DATA_ONLY, 0x06,
	DATA_ONLY, 0x09,
	DATA_ONLY, 0x0d,
	DATA_ONLY, 0x0f,
	DATA_ONLY, 0x12,
	DATA_ONLY, 0x15,
	DATA_ONLY, 0x18,

	0xb2, 0x10,
	DATA_ONLY, 0x10,
	DATA_ONLY, 0x0b,
	DATA_ONLY, 0x05,

	ENDDEF, 0x0000
};

static const unsigned short seq_acl_on[] = {
	/* ACL on */
	0xc0, 0x01,

	ENDDEF, 0x0000
};

static const unsigned short seq_elvss_on[] = {
	/* ELVSS on */
	0xb1, 0x0b,

	ENDDEF, 0x0000
};

static const unsigned short seq_stand_by_off[] = {
	0x11, COMMAND_ONLY,

	ENDDEF, 0x0000
};

static const unsigned short seq_display_on[] = {
	0x29, COMMAND_ONLY,

	ENDDEF, 0x0000
};

static const unsigned int s6e63m0_22_300[] = {
	0x18, 0x08, 0x24, 0x5f, 0x50, 0x2d, 0xB6,
	0xB9, 0xA7, 0xAd, 0xB1, 0x9f, 0xbe, 0xC0,
	0xB5, 0x00, 0xa0, 0x00, 0xa4, 0x00, 0xdb
};

static int s6e63m0_spi_write_byte(struct s6e63m0_panel_priv *priv, int addr, int data)
{
	u16 buf[1];

	buf[0] = (addr << 8) | data;

	return spi_xfer(priv->spi, 16, buf, NULL, 0); 
}

static int s6e63m0_spi_write(struct s6e63m0_panel_priv *priv, unsigned char address,
	unsigned char command)
{
	int ret = 0;

	if (address != DATA_ONLY)
		ret = s6e63m0_spi_write_byte(priv, 0x0, address);
	if (command != COMMAND_ONLY)
		ret = s6e63m0_spi_write_byte(priv, 0x1, command);

	return ret;
}

static int s6e63m0_panel_send_sequence(struct s6e63m0_panel_priv *priv,
	const unsigned short *wbuf)
{
	int ret = 0, i = 0;

	while ((wbuf[i] & DEFMASK) != ENDDEF) {
		if ((wbuf[i] & DEFMASK) != SLEEPMSEC) {
			ret = s6e63m0_spi_write(priv, wbuf[i], wbuf[i+1]);
			if (ret)
				break;
		} else {
			udelay(wbuf[i+1] * 1000);
		}
		i += 2;
	}

	return ret;
}

static int s6e63m0_ldi_init(struct s6e63m0_panel_priv *priv)
{
	int ret, i;
	const unsigned short *init_seq[] = {
		seq_panel_condition_set,
		seq_display_condition_set,
		seq_gamma_setting,
		seq_etc_condition_set,
		seq_acl_on,
		seq_elvss_on,
	};

	for (i = 0; i < ARRAY_SIZE(init_seq); i++) {
		ret = s6e63m0_panel_send_sequence(priv, init_seq[i]);
		if (ret)
			break;
	}

	return ret;
}

static int s6e63m0_ldi_enable(struct s6e63m0_panel_priv *priv)
{
	int ret = 0, i;
	const unsigned short *enable_seq[] = {
		seq_stand_by_off,
		seq_display_on,
	};

	for (i = 0; i < ARRAY_SIZE(enable_seq); i++) {
		ret = s6e63m0_panel_send_sequence(priv, enable_seq[i]);
		if (ret)
			break;
	}

	return ret;
}

static int s6e63m0_gamma_ctl(struct s6e63m0_panel_priv *priv)
{
	unsigned int i = 0;
	int ret = 0;

	/* disable gamma table updating. */
	ret = s6e63m0_spi_write(priv, 0xfa, 0x00);
	if (ret) {
		debug("failed to disable gamma table updating.\n");
		goto gamma_err;
	}

	for (i = 0 ; i < ARRAY_SIZE(s6e63m0_22_300); i++) {
		ret = s6e63m0_spi_write(priv, DATA_ONLY, s6e63m0_22_300[i]);
		if (ret) {
			debug("failed to set gamma table.\n");
			goto gamma_err;
		}
	}

	/* update gamma table. */
	ret = s6e63m0_spi_write(priv, 0xfa, 0x01);
	if (ret)
		debug("failed to update gamma table.\n");

gamma_err:
	return ret;
}

static int s6e63m0_panel_enable_backlight(struct udevice *dev)
{
	struct s6e63m0_panel_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s: start\n", __func__);
	dm_gpio_set_value(&priv->reset, 0);
	udelay(10000);
	dm_gpio_set_value(&priv->reset, 1);
	udelay(10000);
	dm_gpio_set_value(&priv->reset, 0);
	udelay(10000);

	ret = s6e63m0_ldi_init(priv);
	if (ret) {
		debug("failed to initialize ldi.\n");
		return ret;
	}

	ret = s6e63m0_ldi_enable(priv);
	if (ret) {
		debug("failed to enable ldi.\n");
		return ret;
	}

	/* set brightness to current value after power on or resume. */
	ret = s6e63m0_gamma_ctl(priv);
	if (ret) {
		debug("s6e63m0 gamma setting failed.\n");
		return ret;
	}

	debug("%s: done, ret = %d\n", __func__, ret);

	return ret;
}

static int s6e63m0_panel_ofdata_to_platdata(struct udevice *dev)
{
	struct s6e63m0_panel_priv *priv = dev_get_priv(dev);
	int ret;

	if (IS_ENABLED(CONFIG_DM_REGULATOR)) {
		ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
						   "vdd3-supply", &priv->vdd3);
		if (ret) {
			debug("%s: Warning: cannot get vdd3 power supply: ret=%d\n",
			      __func__, ret);
			return ret;
		}

		ret = uclass_get_device_by_phandle(UCLASS_REGULATOR, dev,
						   "vci-supply", &priv->vci);
		if (ret) {
			debug("%s: Warning: cannot get vci power supply: ret=%d\n",
			      __func__, ret);
			return ret;
		}
	}
	ret = gpio_request_by_name(dev, "reset-gpios", 0, &priv->reset,
				   GPIOD_IS_OUT);
	if (ret) {
		debug("%s: Warning: cannot get reset GPIO: ret=%d\n",
		      __func__, ret);
		return ret;
	}

	pr_err("Got here\n");

	env_set("s6e63m0", "of_pdata_called");

	return 0;
}

static int s6e63m0_panel_probe(struct udevice *dev)
{
	struct s6e63m0_panel_priv *priv = dev_get_priv(dev);
	int ret;

	if (IS_ENABLED(CONFIG_DM_REGULATOR) && priv->vdd3) {
		debug("%s: Enable regulator '%s'\n", __func__, priv->vdd3->name);
		ret = regulator_set_enable(priv->vdd3, true);
		if (ret)
			return ret;
		debug("%s: Enable regulator '%s'\n", __func__, priv->vci->name);
		ret = regulator_set_enable(priv->vci, true);
		if (ret)
			return ret;
	}

	priv->spi = dev_get_parent_priv(dev);

	ret = spi_claim_bus(priv->spi);
	if (ret)
		return ret;

	return 0;
}

static const struct panel_ops s6e63m0_panel_ops = {
	.enable_backlight	= s6e63m0_panel_enable_backlight,
};

static const struct udevice_id s6e63m0_panel_ids[] = {
	{ .compatible = "samsung,s6e63m0" },
	{ }
};

U_BOOT_DRIVER(s6e63m0_panel) = {
	.name	= "s6e63m0_panel",
	.id	= UCLASS_PANEL,
	.of_match = s6e63m0_panel_ids,
	.ops	= &s6e63m0_panel_ops,
	.ofdata_to_platdata	= s6e63m0_panel_ofdata_to_platdata,
	.probe		= s6e63m0_panel_probe,
	.priv_auto_alloc_size	= sizeof(struct s6e63m0_panel_priv),
};
