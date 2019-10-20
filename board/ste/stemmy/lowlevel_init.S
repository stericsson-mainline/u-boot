/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Stephan Gerhold
 *
 * Based on rpi/lowlevel_init.S:
 * Copyright (C) 2016 Cédric Schieli <cschieli@gmail.com>
 */

#include <config.h>

.align 8
.global fw_machid
fw_machid:
	.word 0x0
.global fw_atags
fw_atags:
	.word 0x0

/*
 * Routine: save_boot_params (called after reset from start.S)
 * Description: save machid and ATAG/FDT address provided by the firmware
 */

.global save_boot_params
save_boot_params:

	/* The firmware provided ATAG address can be found in r2 */
	str	r1, fw_machid
	str	r2, fw_atags

	/* Returns */
	b	save_boot_params_ret
