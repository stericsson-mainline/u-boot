/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2019 Stephan Gerhold <stephan@gerhold.net>
 */
#ifndef __CONFIGS_STEMMY_H
#define __CONFIGS_STEMMY_H

#include <linux/sizes.h>

/*
 * The "stemmy" U-Boot port is designed to be chainloaded by the Samsung
 * bootloader on devices based on ST-Ericsson Ux500. Therefore, we skip most
 * low-level initialization and rely on configuration provided by the Samsung
 * bootloader. New images are loaded at the same address for compatibility.
 */

/* FIXME: This should be loaded from device tree... */
#define CFG_SYS_PL310_BASE		0xa0412000

/* Linux does not boot if FDT / initrd is loaded to end of RAM */
#define BOOT_ENV \
	"fdt_high=0x6000000\0" \
	"initrd_high=0x6000000\0"

#define CONSOLE_ENV \
	"stdin=serial\0" \
	"stdout=serial,vidconsole\0" \
	"stderr=serial,vidconsole\0"

#define FASTBOOT_ENV \
	"fastboot_partition_alias_u-boot=Kernel\0" \
	"fastboot_partition_alias_boot=Kernel\0" \
	"fastboot_partition_offset_boot=0x800\0" \
	"fastboot_partition_alias_recovery=Kernel2\0" \
	"fastboot_partition_alias_system=SYSTEM\0" \
	"fastboot_partition_alias_cache=CACHEFS\0" \
	"fastboot_partition_alias_hidden=HIDDEN\0" \
	"fastboot_partition_alias_userdata=DATAFS\0"

#define BOOTCMD_ENV \
	"partitionbootcmd=" \
		"abootimgaddr=0x18100000;" /* TODO: XIP Kernel Image? */ \
		"if part start mmc 0 $bootpart boot_start; then " \
			"part size mmc 0 $bootpart boot_size;" \
			"setexpr boot_start $boot_start + ${bootpartoffset:-0};" \
			"setexpr boot_size $boot_size - ${bootpartoffset:-0};" \
			"mmc read $abootimgaddr $boot_start $boot_size;" \
			"bootm $abootimgaddr;" \
		"else " \
			"echo Partition $bootpart not found;" \
		"fi;" \
		"echo Boot failed, starting fastboot mode...;" \
		"run fastbootcmd\0" \
	"androidbootcmd=" \
		"setenv bootpart ${fastboot_partition_alias_boot:-boot};" \
		"setenv bootpartoffset ${fastboot_partition_offset_boot};" \
		"run partitionbootcmd\0" \
	"recoverybootcmd=" \
		"setenv bootpart ${fastboot_partition_alias_recovery:-recovery};" \
		"setenv bootpartoffset ${fastboot_partition_offset_recovery};" \
		"echo Booting into recovery...;" \
		"run partitionbootcmd\0" \
	"fastbootcmd=echo '*** FASTBOOT MODE ***'; fastboot usb 0\0"

#define CFG_EXTRA_ENV_SETTINGS \
	BOOT_ENV \
	CONSOLE_ENV \
	FASTBOOT_ENV \
	BOOTCMD_ENV

#endif
