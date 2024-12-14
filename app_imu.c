#include <includes.h>

/*
*********************************************************************************************************
*                                             DEFINES
*********************************************************************************************************
*/

#define TASK_STK_SIZE 512 /* Size of each task's stacks (# of bytes)          */
#define N_TASKS 20		  /* Number of identical tasks                        */

#define TASK_START_ID 0 /* Application tasks   		                      */
#define TASK_1_ID 1
#define TASK_2_ID 2
#define TASK_3_ID 3
#define TASK_4_ID 4
#define TASK_5_ID 5
#define TASK_6_ID 6
// #define	TASK_7_ID           7
// #define	TASK_8_ID           8
// #define	TASK_9_ID           9

#define TASK_START_PRIO 10 /* Application tasks priorities                  */
#define TASK_1_PRIO 11
#define TASK_2_PRIO 12
#define TASK_3_PRIO 13
#define TASK_4_PRIO 14
#define TASK_5_PRIO 15
#define TASK_6_PRIO 16
// #define	TASK_7_PRIO        	17
// #define	TASK_8_PRIO        	18
// #define	TASK_9_PRIO        	19

#define MSG_QUEUE_SIZE 20 /* Size of message queue used in example         */

/*
*********************************************************************************************************
*                                             DATA TYPES
*********************************************************************************************************
*/

typedef struct
{
	char TaskName[30];
	INT16U TaskCtr;
	INT16U TaskExecTime;
	INT32U TaskTotExecTime;
} TASK_USER_DATA;

/*
**************************************************************************************************************
*                                               VARIABLES
**************************************************************************************************************
*/
OS_STK TaskStartStk[TASK_STK_SIZE]; /* Startup    task stack                         */
OS_STK Task1Stk[TASK_STK_SIZE];		/* Task #1    task stack                         */
OS_STK Task2Stk[TASK_STK_SIZE];		/* Task #2    task stack                         */
OS_STK Task3Stk[TASK_STK_SIZE];		/* Task #3    task stack                         */
OS_STK Task4Stk[TASK_STK_SIZE];
OS_STK Task5Stk[TASK_STK_SIZE];
OS_STK Task6Stk[TASK_STK_SIZE];

OS_EVENT *LCDSem;

char TaskData[N_TASKS];

TASK_USER_DATA TaskUserData[N_TASKS];

OS_EVENT *MsgQueue;	   /* Message queue pointer                         */
void *MsgQueueTbl[20]; /* Storage for messages                          */

extern __IO uint16_t ADCConvertedValue[5];

/*
**************************************************************************************************************
* 									FUNCTION PROTOTYPES
**************************************************************************************************************
*/

void TaskStart(void *data); /* Function prototypes of Startup task              */
void Task1(void *data);		/* Function prototypes of tasks*/
void Task2(void *data);		/* Function prototypes of tasks*/
void Task3(void *data);		/* Function prototypes of tasks*/
void Task4(void *data);		/* Function prototypes of tasks*/

void Delay(void);
void Beep(void);
void longBeep(void);

/*
**************************************************************************************************************
*                                                MAIN
**************************************************************************************************************
*/
int main(void)
{

	INT8U os_err = 0;

	os_err = os_err;

	BSP_Init();

	// delay for LCD init.
	DelayMS(300);
	GLCD_init();
	GLCD_clear();

	GLCD_xy(0, 0);
	printf("Real-time Sensor Kit");
	GLCD_xy(2, 0);
	printf("     x     y     z   ");
	GLCD_xy(3, 0);
	printf("XL :                 ");
	GLCD_xy(4, 0);
	printf("GY :                 ");
	GLCD_xy(5, 0);
	printf("AVG:                 ");

	IMU_Init();
	GLCD_xy(7, 0);
	printf("-------Running-------");

	OSInit();

	KeyInit();

	os_err = OSTaskCreate(TaskStart, (void *)0, (void *)&TaskStartStk[TASK_STK_SIZE - 1], 0);

	OSStart(); /* Start multitasking                               */

	return 0;
}

/*
**************************************************************************************************************
*                                              STARTUP TASK
**************************************************************************************************************
*/

void TaskStart(void *data)
{
	OS_CPU_SR cpu_sr;

	data = data; /* Prevent compiler warning                         */

	OS_ENTER_CRITICAL();
	SysTick_Configuration(); // 1ms systick
	OS_EXIT_CRITICAL();

	OSStatInit(); // Initialize uC/OS-II's statistics

	LCDSem = OSSemCreate(1);
	CommInit();

	OSTaskCreate(Task1, (void *)0, (void *)&Task1Stk[TASK_STK_SIZE - 1], TASK_1_PRIO);
	OSTaskCreate(Task2, (void *)0, (void *)&Task2Stk[TASK_STK_SIZE - 1], TASK_2_PRIO);
	OSTaskCreate(Task3, (void *)0, (void *)&Task3Stk[TASK_STK_SIZE - 1], TASK_3_PRIO);
	OSTaskCreate(Task4, (void *)0, (void *)&Task4Stk[TASK_STK_SIZE - 1], TASK_4_PRIO);

	for (;;)
	{

		OSCtxSwCtr = 0;
		OSTimeDlyHMSM(0, 0, 1, 0); //* Wait one second
	}
}

/*
**************************************************************************************************************
*                                                  TASKS
**************************************************************************************************************
*/
void Task1(void *data)
{
	data = data;

	for (;;)
	{

		LED_TOGGLE(0);
		OSTimeDlyHMSM(0, 0, 0, 200);
	}
}

void Task2(void *data)
{
	data = data;

	for (;;)
	{

		LED_TOGGLE(1);
		OSTimeDlyHMSM(0, 0, 0, 500);
	}
}

void Task3(void *data)
{
	data = data;

	for (;;)
	{

		LED_TOGGLE(2);
		OSTimeDlyHMSM(0, 0, 1, 0);
	}
}

void Task4(void *data)
{
	INT8U err = 0;
	err = err;
	data = data;

	uint8_t temp[16];

	int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;

	float OutX_G, OutY_G, OutZ_G;
	float OutX_XL, OutY_XL, OutZ_XL;

	for (;;)
	{

		IMU_I2C_BufferRead(IMU_LSM6DS3, temp, 0x22, 12);

		GyX = (int16_t)((uint16_t)temp[1] << 8 | temp[0]);
		GyY = (int16_t)((uint16_t)temp[3] << 8 | temp[2]);
		GyZ = (int16_t)((uint16_t)temp[5] << 8 | temp[4]);

		AcX = (int16_t)((uint16_t)temp[7] << 8 | temp[6]);
		AcY = (int16_t)((uint16_t)temp[9] << 8 | temp[8]);
		AcZ = (int16_t)((uint16_t)temp[11] << 8 | temp[10]);

		// 16g / max range
		OutX_G = ((float)GyX / 32767.) * 16;
		OutY_G = ((float)GyY / 32767.) * 16;
		OutZ_G = ((float)GyZ / 32767.) * 16;

		// 2000dps / max range
		OutX_XL = ((float)AcX / 32767.) * 2000;
		OutY_XL = ((float)AcY / 32767.) * 2000;
		OutZ_XL = ((float)AcZ / 32767.) * 2000;

		LED_TOGGLE(3);

		OSSemPend(LCDSem, 0, &err); // Acquire semaphore for using charater LCD

		GLCD_xy(3, 4);
		printf("%5d %5d %5d", (int16_t)OutX_XL, (int16_t)OutY_XL, (int16_t)OutZ_XL);
		GLCD_xy(4, 4);
		printf("%5d %5d %5d", (int16_t)OutX_G, (int16_t)OutY_G, (int16_t)OutZ_G);

		float avg = (OutX_XL + OutY_XL + OutZ_XL) / 3.0;
		GLCD_xy(5, 4);
		printf("%5d", (int16_t)avg);

		OSSemPost(LCDSem);

		if (avg > 70)
		{
			longBeep();
		}
		else if (avg < 30)
		{
			longBeep();
		}
		else
		{
		}
	}
}

void Beep(void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_15);
	OSTimeDlyHMSM(0, 0, 0, 100);
	GPIO_ResetBits(GPIOD, GPIO_Pin_15);
}

// ���� �Լ�
void longBeep(void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_15);
	OSTimeDlyHMSM(0, 0, 0, 500);
	GPIO_ResetBits(GPIOD, GPIO_Pin_15);
}

/**
 * @brief  Delay
 * @param  None
 * @retval None
 */
void Delay(void)
{
	uint16_t nTime = 0x0000;

	for (nTime = 0; nTime < 0x0FFF; nTime++)
	{
	}
}

#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	   ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}

#endif