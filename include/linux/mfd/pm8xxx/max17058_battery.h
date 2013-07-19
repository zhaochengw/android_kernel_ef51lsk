#ifndef __MAX17058_BATTERY_H
#define __MAX17058_BATTERY_H

#if defined(CONFIG_PANTECH_MAX17058_FG)
void max17058_set_rcomp(u8 msb, u8 lsb);
int max17058_calc_rcomp(unsigned int batt_temp);
int max17058_get_vcell(void);
int max17058_get_soc(void);
#else
static inline void max17058_set_rcomp(u8 msb, u8 lsb)
{
}
static inline int max17058_calc_rcomp(unsigned int batt_temp)
{
}
static inline int max17058_get_vcell(void)
{
	return -ENXIO;
}
static inline int max17058_get_soc(void)
{
	return -ENXIO;
}
#endif

#endif
