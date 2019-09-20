// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Stephan Gerhold
 */

#include <common.h>
#include <asm/armv7.h>
#include <asm/pl310.h>

#define PL310_WAY_MASK	0xff

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

#ifdef CONFIG_SYS_L2_PL310
void v7_outer_cache_disable(void)
{
	struct pl310_regs *const pl310 = (struct pl310_regs *)CONFIG_SYS_PL310_BASE;

	/*
	 * Linux expects the L2 cache to be turned off by the bootloader.
	 * Otherwise, it fails very early shortly after decompressing the kernel.
	 *
	 * On U8500, the L2 cache can be only turned on/off from the secure world.
	 * Therefore, the only choice we have is to lock all ways of the L2 cache,
	 * to prevent it from being used.
	 */
	writel(PL310_WAY_MASK, &pl310->pl310_lockdown_dbase);
	writel(PL310_WAY_MASK, &pl310->pl310_lockdown_ibase);
}
#endif
