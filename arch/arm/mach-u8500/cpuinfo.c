// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Stephan Gerhold
 *
 * Based on old mainline u-boot implementation:
 * Copyright (C) 2012 Linaro Limited
 * Mathieu Poirier <mathieu.poirier@linaro.org>
 *
 * Based on original code from Joakim Axelsson at ST-Ericsson
 * Copyright (C) 2010 ST-Ericsson
 */

#include <common.h>
#include <asm/io.h>

#define CPUID_DB8500ED	0x410fc090
#define CPUID_DB8500V1	0x411fc091
#define CPUID_DB8500V2	0x412fc091

#define U8500_BOOTROM_BASE	0x90000000
#define U8500_ASIC_ID_LOC_ED_V1	(U8500_BOOTROM_BASE + 0x1FFF4)
#define U8500_ASIC_ID_LOC_V2	(U8500_BOOTROM_BASE + 0x1DBF4)

static inline uint read_cpuid(void)
{
	uint val;

	/* Main ID register (MIDR) */
	asm("mrc	p15, 0, %0, c0, c0, 0"
	   : "=r" (val)
	   :
	   : "cc");

	return val;
}

static uint read_asicid(void) {
	switch (read_cpuid()) {
	case CPUID_DB8500ED:
	case CPUID_DB8500V1:
		return readl(U8500_ASIC_ID_LOC_ED_V1);
	default:
		return readl(U8500_ASIC_ID_LOC_V2);
	}
}

int print_cpuinfo(void)
{
	/* ASIC ID 0x8500A0 => DB8520 V1.0 */
	uint asicid = read_asicid();
	uint cpu = (asicid >> 8) & 0xffff;
	uint rev = asicid & 0xff;

	/* 0xA0 => V1.0 */
	if (rev >= 0xa0)
		rev -= 0x90;

	printf("CPU: ST-Ericsson DB%x V%d.%d\n", cpu, rev >> 4, rev & 0xf);
	return 0;
}
