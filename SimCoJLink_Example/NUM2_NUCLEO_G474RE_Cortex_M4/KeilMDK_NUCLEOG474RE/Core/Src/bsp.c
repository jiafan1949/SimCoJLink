#include "bsp.h"
#include "Subsystem.h"
#include "SEGGER_RTT.h"


static void StopPWM(void);

static void UploadParameter(void);

uint16_t Adc1ValueIN1, Adc1ValueIN6, Adc2ValueIN7;
uint32_t Temperature, Vbus;
float encoder_angle, encoder_angle_1;
volatile enMcErr mc_err;
enMcState mc_state;
stMcCMD mc_cmd;


float CmdFreq, CmdAmp;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	Subsystem_step();
	UploadParameter();
}
/*
 *  Function Describe : Deal with FOC algorithm.
 *  exec period : 20kHz
 */
void HAL_ADCEx_InjectedConvCpltCallback( ADC_HandleTypeDef *hadc)
{
	extern ADC_HandleTypeDef hadc1, hadc2;
	extern TIM_HandleTypeDef htim1;
	extern TIM_HandleTypeDef htim2;

	if( hadc->Instance == hadc1.Instance)
	{
//		UploadParameter();
	}
}


/*
 *  Function Describe : When over current, shutdown pwm output. 
 *  
 */
void HAL_TIMEx_Break2Callback( TIM_HandleTypeDef * htim)
{
	extern TIM_HandleTypeDef htim1;
	HAL_TIM_PWM_Stop( &htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop( &htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Stop( &htim1, TIM_CHANNEL_3);
	HAL_TIMEx_PWMN_Stop_IT( &htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Stop_IT( &htim1, TIM_CHANNEL_2);
	HAL_TIMEx_PWMN_Stop_IT( &htim1, TIM_CHANNEL_3);

	mc_err = OC_ERR;
	HAL_GPIO_WritePin( GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
}

int32_t encoder_count;
uint16_t encoder_dir;
float z_count;
// z signal angle: 2.337 rad.
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	extern TIM_HandleTypeDef htim2;
	if( GPIO_Pin == GPIO_PIN_10)
	{
//		encoder_dir = __HAL_TIM_IS_TIM_COUNTING_DOWN( &htim2);
//		encoder_count = __HAL_TIM_GET_COUNTER( &htim2) - 65536;
//		__HAL_TIM_SET_COUNTER( &htim2, 65536);
		encoder_angle = 0.5155366f;
	}
}

void BspCordicInit(void)
{

}

void StopPWM(void)
{
	extern TIM_HandleTypeDef htim1;
	HAL_TIM_PWM_Stop( &htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop( &htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Stop( &htim1, TIM_CHANNEL_3);
	HAL_TIMEx_PWMN_Stop_IT( &htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Stop_IT( &htim1, TIM_CHANNEL_2);
	HAL_TIMEx_PWMN_Stop_IT( &htim1, TIM_CHANNEL_3);
}

void StartPWM(void)
{
	extern TIM_HandleTypeDef htim1;
	HAL_TIM_PWM_Start( &htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start( &htim1, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start( &htim1, TIM_CHANNEL_3);
	HAL_TIMEx_PWMN_Start_IT( &htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start_IT( &htim1, TIM_CHANNEL_2);
	HAL_TIMEx_PWMN_Start_IT( &htim1, TIM_CHANNEL_3);
}

void BspInit(void)
{
	mc_cmd.cmd = STOP_CMD;
	mc_cmd.refFreq = 0.0f;
	mc_cmd.refIq = 0.0f;
	mc_cmd.refRPM = 0.0f;
	mc_cmd.refPos = 0.0f;
	mc_state = MC_RDY;
	mc_err = NONE_ERR;
}

float TransBuff[20];
uint8_t CmdBuffer[100];
float CmdType[20];
int16_t AvailBytes, AvailReadBytes;
uint32_t RecvCount;
uint8_t vv_debug;
float vv_debug_float;
float KKK_Freq;
void UploadParameter(void)
{
	static uint16_t count;
	uint8_t TransHeader[] = { 'T','T',':'};
	uint8_t TransTerminator[] = {'E', 'T', '!'};
	static float kk_test = 0.0F;
	if(kk_test > 10.0F)
		kk_test = 0.0F;
	kk_test += 0.05F;

	TransBuff[0] = Observer.theta_e;
	TransBuff[1] = CmdType[0] * Observer.u_alpha;
	TransBuff[2] = CmdType[0] * Observer.u_beta;
//	TransBuff[3] = Observer.theta_e;
//	TransBuff[4] = Observer.u_alpha;
//	TransBuff[5] = Observer.u_beta;
//	TransBuff[6] = Observer.theta_e;
//	TransBuff[7] = Observer.u_alpha;
//	TransBuff[8] = Observer.u_beta;
//	TransBuff[9] = Observer.theta_e;
//	TransBuff[10] = Observer.u_alpha;
//	TransBuff[11] = Observer.u_beta;
//	TransBuff[12] = Observer.theta_e;
//	TransBuff[13] = Observer.u_alpha;
//	TransBuff[14] = Observer.u_beta;
//	TransBuff[15] = Observer.theta_e;
//	TransBuff[16] = Observer.u_alpha;
//	TransBuff[17] = Observer.u_beta;
//	TransBuff[18] = Observer.theta_e;
//	TransBuff[19] = Observer.u_alpha;
//	TransBuff[20] = Observer.u_beta;

	if( count == 0)
	{
		SEGGER_RTT_WriteNoLock(0, TransHeader, 3);
		SEGGER_RTT_WriteNoLock(0, TransBuff, 3*4);
	}
	else if( count == 999)
	{
		SEGGER_RTT_WriteNoLock(0, TransBuff, 3*4);
		SEGGER_RTT_WriteNoLock(0, TransTerminator, 3);
	}
	else
	{
		SEGGER_RTT_WriteNoLock(0, TransBuff, 3*4);
	}

	if( count == 999)
	{
		count = 0;
	}
	else
	{
		count++;
	}
	AvailBytes = SEGGER_RTT_GetAvailWriteSpace(0);
	AvailReadBytes = SEGGER_RTT_GetAvailReadSpace(0);
	
	if(AvailReadBytes > 25-1)
	{
		SEGGER_RTT_ReadNoLock(0, CmdBuffer, 25);
		static float * CmdTypePtr = (float*)(CmdBuffer + 3);
		RecvCount++;
		memcpy(CmdType, CmdBuffer+3, 20);
		KKK_Freq = CmdType[1];
//		vv_debug_float = CmdTypePtr[0];
//		a = *(float *)(CmdBuffer + 3);
//		b = *(float *)(CmdBuffer + 3) + 1;
//		c = *(CmdTypePtr+2);
//		d = *(CmdTypePtr+3);
	}
}

