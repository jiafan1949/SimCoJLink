#include "stm32g4xx_hal.h"

#define PI_2		6.2832F

#define PC10_SET		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET)
#define PC10_RESET		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET)

#define PC11_SET		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_SET)
#define PC11_RESET		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_11, GPIO_PIN_RESET)

#define PC12_SET		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET)
#define PC12_RESET		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET)

#define PD2_SET			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_SET)
#define PD2_RESET		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET)

typedef enum
{
	NONE_ERR,
	LV_ERR,
	OV_ERR,
	OC_ERR
} enMcErr;

typedef enum
{
	MC_RDY,
	MC_STAT,
	MC_RUN,
	MC_STOP,
	MC_ERR
} enMcState;

typedef enum
{
	STAT_CMD,
	STOP_CMD
} enMcCMD;

typedef struct
{
	enMcCMD cmd;
	float refFreq;
	float refIq;
	float refRPM;
	float refPos;
} stMcCMD;


extern void StartPWM(void);
extern void BspCordicInit(void);
extern void BspInit(void);
