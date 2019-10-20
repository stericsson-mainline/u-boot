// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Stephan Gerhold <stephan@gerhold.net>
 */
#include <common.h>
#include <env.h>
#include <init.h>
#include <stdlib.h>
#include <asm/gpio.h>
#include <asm/setup.h>

DECLARE_GLOBAL_DATA_PTR;

extern uint fw_machid;
extern struct tag *fw_atags;

static struct tag *fw_atags_copy = NULL;
static uint fw_atags_size;

int dram_init(void)
{
	/* TODO: Consider parsing ATAG_MEM instead */
	gd->ram_size = get_ram_size(CONFIG_SYS_SDRAM_BASE, CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

int board_init(void)
{
	gd->bd->bi_arch_number = fw_machid;
	gd->bd->bi_boot_params = (ulong) fw_atags;
	return 0;
}

struct gpio_keys {
	struct gpio_desc vol_up;
	struct gpio_desc vol_down;
};

static void request_gpio_key(int node, const char *name, struct gpio_desc *desc)
{
	int ret;

	if (node < 0)
		return;

	ret = gpio_request_by_name_nodev(offset_to_ofnode(node), "gpios", 0,
					 desc, GPIOD_IS_IN);
	if (ret) {
		printf("Failed to request %s GPIO: %d\n", name, ret);
	}
}

static void request_gpio_keys(const void *fdt, struct gpio_keys *keys)
{
	int offset;
	int vol_up_node = -FDT_ERR_NOTFOUND;
	int vol_down_node = -FDT_ERR_NOTFOUND;

	/* Look for volume-up and volume-down subnodes of gpio-keys */
	offset = fdt_node_offset_by_compatible(fdt, -1, "gpio-keys");
	while (offset != -FDT_ERR_NOTFOUND) {
		if (vol_up_node < 0)
			vol_up_node = fdt_subnode_offset(fdt, offset, "volume-up");
		if (vol_down_node < 0)
			vol_down_node = fdt_subnode_offset(fdt, offset, "volume-down");

		if (vol_up_node >= 0 && vol_down_node >= 0)
			break;

		offset = fdt_node_offset_by_compatible(fdt, offset, "gpio-keys");
	}

	request_gpio_key(vol_up_node, "volume-up", &keys->vol_up);
	request_gpio_key(vol_down_node, "volume-down", &keys->vol_down);
}

static void check_keys(const void *fdt)
{
	struct gpio_keys keys = {0};

	if (!fdt)
		return;

	/* Request gpio-keys from device tree */
	request_gpio_keys(fdt, &keys);

	/* Boot into recovery? */
	if (dm_gpio_get_value(&keys.vol_up) == 1) {
		env_set("bootcmd", "run recoverybootcmd");
	}

	/* Boot into fastboot? */
	if (dm_gpio_get_value(&keys.vol_down) == 1) {
		env_set("preboot", "setenv preboot; run fastbootcmd");
	}
}

/*
 * The downstream/vendor kernel (provided by Samsung) uses atags for booting.
 * It also requires an extremely long cmdline provided by the primary bootloader
 * that is not suitable for booting mainline.
 *
 * Since downstream is the only user of atags, we emulate the behavior of the
 * Samsung bootloader by generating only the initrd atag in u-boot, and copying
 * all other atags as-is from the primary bootloader.
 */
static inline bool skip_atag(u32 tag)
{
	return (tag == ATAG_NONE || tag == ATAG_CORE ||
		tag == ATAG_INITRD || tag == ATAG_INITRD2);
}

static void parse_serial(struct tag_serialnr *serialnr)
{
	char serial[17];

	if (env_get("serial#"))
		return;

	sprintf(serial, "%08x%08x", serialnr->high, serialnr->low);
	env_set("serial#", serial);
}

static void copy_atags(struct tag *tags)
{
	struct tag *t, *copy;

	if (tags->hdr.tag != ATAG_CORE) {
		printf("Invalid atags provided by primary bootloader: "
		       "tag 0x%x at 0x%px\n", tags->hdr.tag, tags);
		return;
	}

	fw_atags_size = 0;

	/* Calculate necessary size for tags we want to copy */
	for_each_tag(t, tags) {
		if (skip_atag(t->hdr.tag))
			continue;

		if (t->hdr.tag == ATAG_SERIAL)
			parse_serial(&t->u.serialnr);

		fw_atags_size += t->hdr.size << 2;
	}

	if (!fw_atags_size)
		return;  /* No tags to copy */

	fw_atags_copy = malloc(fw_atags_size);
	if (!fw_atags_copy)
		return;
	copy = fw_atags_copy;

	/* Copy tags */
	for_each_tag(t, tags) {
		if (skip_atag(t->hdr.tag))
			continue;

		memcpy(copy, t, t->hdr.size << 2);
		copy = tag_next(copy);
	}
}

int misc_init_r(void)
{
	check_keys(gd->fdt_blob);
	copy_atags(fw_atags);
	return 0;
}

void setup_board_tags(struct tag **in_params)
{
	u8 **bytes = (u8**) in_params;
	if (!fw_atags_copy)
		return;

	memcpy(*bytes, fw_atags_copy, fw_atags_size);
	*bytes += fw_atags_size;
}
