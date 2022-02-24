#include "stm32g4xx_hal.h"
#include "stm32g4xx_ll_cordic.h"
#include "arm_math.h"

extern void UtilsCordicSinCos(float in, float *sin, float *cos);

extern void UtilsCordicAtan2(float in_x, float in_y, float *out);

extern void UtilsCordicModulus(float in_x, float in_y, float *out);

extern void UtilsCordicExp(float in, float *out);

extern void UtilsCordicNegExp(float in, float *out);
