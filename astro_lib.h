/*
* IncFile1.h
*
* Created: 05/11/2012 4:36:38 PM
*  Author: michal
*/

#ifndef ASTRO_H_
#define ASTRO_H_

#include <stdint.h>

#define CONV_SEC_HR(v)      modulus_i((v) / 3600, 24)
#define CONV_SEC_MIN(v)     modulus_i((v) / 60, 60)
#define CONV_SEC_SEC(v)     modulus_i((v), 60)

#define CONV_ARCSEC_DEG(v)    modulus_i((v) / 3600, 360)
#define CONV_ARCSEC_ARCMIN(v) modulus_i((v) / 60, 60)
#define CONV_ARCSEC_ARCSEC(v) modulus_i((v), 60)

#define CONST_ARCSEC_PER_360  1296000     // num of arcseconds in 360deg
#define CONST_SEC_PER_DAY   86400     // num of seconds in 24h
#define CONST_SIDEREAL_SEC    86164.09053   // number of seconds in sidereal day

#define CONST_DAYS_PER_YEAR   365

#define CONST_GOTO_STOP_THRESHOLD CONV_ARCSEC_TO_ANGLE(10)     // angle error delta when goto mode concludes

#define CONV_RA_DEC_TO_ALT(ra, dec)   ra
#define CONV_RA_DEC_TO_AZ(ra, dec)    dec

#define CONV_ARCSEC_TO_ANGLE(v)     ((v)/3600.0) // 0..1296000 to 0..360
#define CONV_ANGLE_TO_ARCSEC(v)     ((v)*3600.0) // 0..360 to 0..1296000

#define CONV_SEC_TO_ARCSEC(v)     ((v)*15.0) // 0..86400 to 0..1296000
#define CONV_SEC_TO_ANGLE(v)      ((v)/240.0) // 0..86400 to 0..360

#define CONV_ARCSEC_TO_SEC(v)     ((v)/15.0) // 0..1296000 to 0..86400
#define CONV_ANGLE_TO_SEC(v)      ((v)*240.0) // 0..360 to 0..86400

#define CONV_SEC_TO_SIDEREAL(v)     (((v)*CONST_SEC_PER_DAY)/CONST_SIDEREAL_SEC);

int32_t modulus_i(int32_t value, int32_t divisor);
double modulus_d(double value, double divisor);
double two_compl_d(double value, double limit);
int32_t two_compl_i(int32_t value, int32_t limit);
double abs_d(double value);
int8_t sign_i(double value);
double sign_d(double value);
double limit_d(double value, double limit_low, double limit_high);
int32_t limit_i(int32_t value, int32_t limit_low, int32_t limit_high);

#endif /* ASTRO_H_ */
