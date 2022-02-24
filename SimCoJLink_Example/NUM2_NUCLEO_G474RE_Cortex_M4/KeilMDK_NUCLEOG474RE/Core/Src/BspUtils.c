#include "BspUtils.h"
#include "bsp.h"
#include "stm32g4xx_ll_bus.h"

float sin_test, cos_test;

void UtilsCordicSinCos(float in, float *sin, float *cos)
{
	q31_t in_q31, sin_q31, cos_q31;
//	LL_CORDIC_DeInit(CORDIC);
	RCC->AHB1RSTR |= 0x00000008;
	RCC->AHB1RSTR &= 0xFFFFFFF7;
/*	LL_CORDIC_Config(CORDIC, LL_CORDIC_FUNCTION_COSINE,
							 LL_CORDIC_PRECISION_6CYCLES,
							 LL_CORDIC_SCALE_0,
							 LL_CORDIC_NBWRITE_1,
							 LL_CORDIC_NBREAD_2,
							 LL_CORDIC_INSIZE_32BITS,
							 LL_CORDIC_OUTSIZE_32BITS);
	*/
	CORDIC->CSR = 0x00080060;
	float temp = in / PI;
//	arm_float_to_q31( &temp, &in_q31, 1);
	in_q31 = temp * 2147483648.0f;
	LL_CORDIC_WriteData(CORDIC, in_q31);
	cos_q31 = (int32_t)LL_CORDIC_ReadData(CORDIC);
	sin_q31 = (int32_t)LL_CORDIC_ReadData(CORDIC);
//	arm_q31_to_float( &cos_q31, cos, 1);
//	arm_q31_to_float( &sin_q31, sin, 1);
	*cos = cos_q31 / 2147483648.0f;
	*sin = sin_q31 / 2147483648.0f;
	sin_test = *sin;
	cos_test = *cos;
}

void UtilsCordicAtan2(float in_x, float in_y, float *out)
{
	q31_t in_x_q31, in_y_q31, out_q31;
	LL_CORDIC_DeInit(CORDIC);
	LL_CORDIC_Config(CORDIC, LL_CORDIC_FUNCTION_PHASE,
							 LL_CORDIC_PRECISION_6CYCLES,
							 LL_CORDIC_SCALE_0,
							 LL_CORDIC_NBWRITE_2,
							 LL_CORDIC_NBREAD_1,
							 LL_CORDIC_INSIZE_32BITS,
							 LL_CORDIC_OUTSIZE_32BITS);

	arm_float_to_q31( &in_x, &in_x_q31, 1);
	arm_float_to_q31( &in_y, &in_y_q31, 1);
	LL_CORDIC_WriteData(CORDIC, in_x_q31);
	LL_CORDIC_WriteData(CORDIC, in_y_q31);
	out_q31 = (int32_t)LL_CORDIC_ReadData(CORDIC);
	arm_q31_to_float( &out_q31, out, 1);
}


void UtilsCordicModulus(float in_x, float in_y, float *out)
{
	q31_t in_x_q31, in_y_q31, out_q31;
	//LL_CORDIC_DeInit(CORDIC);
	RCC->AHB1RSTR |= 0x00000008;
	RCC->AHB1RSTR &= 0xFFFFFFF7;
/*	LL_CORDIC_Config(CORDIC, LL_CORDIC_FUNCTION_MODULUS,
							 LL_CORDIC_PRECISION_6CYCLES,
							 LL_CORDIC_SCALE_0,
							 LL_CORDIC_NBWRITE_2,
							 LL_CORDIC_NBREAD_1,
							 LL_CORDIC_INSIZE_32BITS,
							 LL_CORDIC_OUTSIZE_32BITS);
	*/
	CORDIC->CSR = 0x00100063;

	in_x /= 20;
	in_y /= 20;
	in_x_q31 = in_x * 2147483648.0f;
	in_y_q31 = in_y * 2147483648.0f;
	LL_CORDIC_WriteData(CORDIC, in_x_q31);
	LL_CORDIC_WriteData(CORDIC, in_y_q31);
	out_q31 = (int32_t)LL_CORDIC_ReadData(CORDIC);
	*out = out_q31 / 2147483648.0f;
	*out *= 20;
}

void UtilsCordicExp(float in, float *out)
{
	q31_t in_q31, sinh_q31, cosh_q31, exp_x_q31;
	LL_CORDIC_DeInit(CORDIC);
	LL_CORDIC_Config(CORDIC, LL_CORDIC_FUNCTION_HCOSINE,
							 LL_CORDIC_PRECISION_6CYCLES,
							 LL_CORDIC_SCALE_0,
							 LL_CORDIC_NBWRITE_1,
							 LL_CORDIC_NBREAD_2,
							 LL_CORDIC_INSIZE_32BITS,
							 LL_CORDIC_OUTSIZE_32BITS);

	arm_float_to_q31( &in, &in_q31, 1);
	LL_CORDIC_WriteData(CORDIC, in_q31);
	cosh_q31 = (int32_t)LL_CORDIC_ReadData(CORDIC);
	sinh_q31 = (int32_t)LL_CORDIC_ReadData(CORDIC);
	exp_x_q31 = cosh_q31 + sinh_q31;
	arm_q31_to_float( &exp_x_q31, out, 1);
}

void UtilsCordicNegExp(float in, float *out)
{
	q31_t in_q31, sinh_q31, cosh_q31, exp_neg_x_q31;
	LL_CORDIC_DeInit(CORDIC);
	LL_CORDIC_Config(CORDIC, LL_CORDIC_FUNCTION_HCOSINE,
							 LL_CORDIC_PRECISION_6CYCLES,
							 LL_CORDIC_SCALE_0,
							 LL_CORDIC_NBWRITE_1,
							 LL_CORDIC_NBREAD_2,
							 LL_CORDIC_INSIZE_32BITS,
							 LL_CORDIC_OUTSIZE_32BITS);

	arm_float_to_q31( &in, &in_q31, 1);
	LL_CORDIC_WriteData(CORDIC, in_q31);
	cosh_q31 = (int32_t)LL_CORDIC_ReadData(CORDIC);
	sinh_q31 = (int32_t)LL_CORDIC_ReadData(CORDIC);
	exp_neg_x_q31 = cosh_q31 - sinh_q31;
	arm_q31_to_float( &exp_neg_x_q31, out, 1);
}
