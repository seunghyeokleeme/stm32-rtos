// 이 파일은 STM32F103VET6 보드에서 uCOS-II RTOS를 사용하여 여러 작업을 수행하는 예제 코드입니다.

#include "includes.h"

// 작업 스택 크기 및 작업 ID 정의
#define TASK_STK_SIZE OS_TASK_DEF_STK_SIZE
#define N_TASKS 10
#define TASK_START_ID 0
#define TASK_CLK_ID 1
#define TASK_1_ID 2
#define TASK_2_ID 3
#define TASK_3_ID 4
#define TASK_4_ID 5
#define TASK_5_ID 6

// 작업 우선 순위 정의
#define TASK_START_PRIO 10
#define TASK_CLK_PRIO 11
#define TASK_1_PRIO 12
#define TASK_2_PRIO 13
#define TASK_3_PRIO 14
#define TASK_4_PRIO 15
#define TASK_5_PRIO 16

// 작업 사용자 데이터 구조체 정의
typedef struct
{
    char TaskName[30];
    INT16U TaskCtr;
    INT16U TaskExecTime;
    INT32U TaskTotExecTime;
} TASK_USER_DATA;

// 전역 변수 선언
OS_EVENT *Sync;
OS_STK TaskStartStk[TASK_STK_SIZE];
OS_STK Task1Stk[TASK_STK_SIZE];
OS_STK Task2Stk[TASK_STK_SIZE];
OS_STK Task3Stk[TASK_STK_SIZE];

// 함수 프로토타입 선언
void TaskStart(void *data);
void Task1(void *data);
void Task2(void *data);
void Task3(void *data);
void Beep(void);
void GenerateDummyIMUData(int32_t *gx, int32_t *gy, int32_t *gz, int32_t *ax, int32_t *ay, int32_t *az, int32_t *mx, int32_t *my, int32_t *mz);

// 메인 함수
int main(void)
{
    INT8U os_err = 0;

    BSP_Init(); // 보드 지원 패키지 초기화

    DelayMS(300);                                    // 300ms 지연
    GLCD_init();                                     // GLCD 초기화
    GLCD_clear();                                    // GLCD 클리어
    LCD_initialize();                                // LCD 초기화
    LCD_string(0x80, (uint8_t *)"STM32-RTOS V.10 "); // LCD에 문자열 출력
    LCD_string(0xC0, (uint8_t *)"Team 8..");

    OSInit();                                                                                 // uCOS-II 초기화
    os_err = OSTaskCreate(TaskStart, (void *)0, (void *)&TaskStartStk[TASK_STK_SIZE - 1], 0); // 시작 작업 생성
    OSStart();                                                                                // OS 시작
    return 0;
}

// 시작 작업 함수
void TaskStart(void *pdata)
{
    OS_CPU_SR cpu_sr;

    pdata = pdata; // 컴파일러 경고 방지

    OS_ENTER_CRITICAL();
    SysTick_Configuration(); // 1ms SysTick 설정
    OS_EXIT_CRITICAL();

    OSStatInit(); // OS 통계 초기화

    Sync = OSSemCreate(0); // 동기화를 위한 세마포어 생성

    // 다른 작업 생성
    OSTaskCreate(Task1, (void *)0, (void *)&Task1Stk[TASK_STK_SIZE - 1], TASK_1_PRIO);
    OSTaskCreate(Task2, (void *)0, (void *)&Task2Stk[TASK_STK_SIZE - 1], TASK_2_PRIO);
    OSTaskCreate(Task3, (void *)0, (void *)&Task3Stk[TASK_STK_SIZE - 1], TASK_3_PRIO);

    for (;;)
    {
        OSCtxSwCtr = 0;
        OSTimeDlyHMSM(0, 0, 1, 0); // 1초 지연
    }
}

// 작업 1 함수
void Task1(void *pdata)
{
    INT8U err;
    pdata = pdata; // 컴파일러 경고 방지
    int32_t gx, gy, gz;
    int32_t ax, ay, az;
    int32_t mx, my, mz;

    for (;;)
    {
        GenerateDummyIMUData(&gx, &gy, &gz, &ax, &ay, &az, &mx, &my, &mz); // 더미 IMU 데이터 생성

        // GLCD에 출력
        GLCD_xy(2, 6);
        printf("GyX: %4d", gx);
        GLCD_xy(2, 12);
        printf("GyY: %4d", gy);
        GLCD_xy(2, 18);
        printf("GyZ: %4d", gz);
        GLCD_xy(2, 24);
        printf("AcX: %4d", ax);
        GLCD_xy(2, 30);
        printf("AcY: %4d", ay);
        GLCD_xy(2, 36);
        printf("AcZ: %4d", az);
        GLCD_xy(2, 42);
        printf("MagX: %4d", mx);
        GLCD_xy(2, 48);
        printf("MagY: %4d", my);
        GLCD_xy(2, 54);
        printf("MagZ: %4d", mz);

        // 가속도 값이 50 이상이면 Beep 함수 호출
        if (ax >= 50 || ay >= 50 || az >= 50)
        {
            Beep();
        }

        // 버튼이 눌리면 Sync 세마포어를 포스트하고 GLCD 클리어
        if (isButtonPressed())
        {
            OSSemPost(Sync);
            GLCD_clear();
        }

        OSTimeDlyHMSM(0, 0, 1, 0); // 1초 지연
    }
}

// 작업 2 함수
void Task2(void *pdata)
{
    INT8U err;
    for (;;)
    {
        OSSemPend(Sync, 0, &err);           // Sync 세마포어 펜딩
        GPIO_SetBits(GPIOD, GPIO_Pin_15);   // 비프 핀 설정
        OSTimeDlyHMSM(0, 0, 0, 50);         // 50ms 지연
        GPIO_ResetBits(GPIOD, GPIO_Pin_15); // 비프 핀 리셋
        OSTimeDlyHMSM(0, 0, 1, 0);          // 1초 지연
    }
}

// 작업 3 함수
void Task3(void *pdata)
{
    {
        LED_TOGGLE(0);               // LED 토글
        OSTimeDlyHMSM(0, 0, 0, 500); // 500ms 지연
    }
}

// 비프 함수
void Beep(void)
{
    GPIO_SetBits(GPIOD, GPIO_Pin_15);   // 비프 핀 설정
    OSTimeDlyHMSM(0, 0, 0, 100);        // 100ms 지연
    GPIO_ResetBits(GPIOD, GPIO_Pin_15); // 비프 핀 리셋
}

// 더미 IMU 데이터 생성 함수
/**
 * @brief 더미 IMU (관성 측정 장치) 데이터를 생성합니다.
 *
 * 이 함수는 자이로스코프, 가속도계 및 자기계에 대한 무작위 더미 데이터를 생성합니다.
 *
 * @param gx 생성된 자이로스코프 X축 데이터 저장을 위한 포인터.
 * @param gy 생성된 자이로스코프 Y축 데이터 저장을 위한 포인터.
 * @param gz 생성된 자이로스코프 Z축 데이터 저장을 위한 포인터.
 * @param ax 생성된 가속도계 X축 데이터 저장을 위한 포인터.
 * @param ay 생성된 가속도계 Y축 데이터 저장을 위한 포인터.
 * @param az 생성된 가속도계 Z축 데이터 저장을 위한 포인터.
 * @param mx 생성된 자기계 X축 데이터 저장을 위한 포인터.
 * @param my 생성된 자기계 Y축 데이터 저장을 위한 포인터.
 * @param mz 생성된 자기계 Z축 데이터 저장을 위한 포인터.
 */
void GenerateDummyIMUData(int32_t *gx, int32_t *gy, int32_t *gz, int32_t *ax, int32_t *ay, int32_t *az, int32_t *mx, int32_t *my, int32_t *mz)
{
    *gx = rand() % 20000 - 10000;
    *gy = rand() % 20000 - 10000;
    *gz = rand() % 20000 - 10000;
    *ax = rand() % 16000 - 8000;
    *ay = rand() % 16000 - 8000;
    *az = rand() % 16000 - 8000;
    *mx = rand() % 4000 - 2000;
    *my = rand() % 4000 - 2000;
    *mz = rand() % 4000 - 2000;
}

// 버튼이 눌렸는지 확인하는 함수
int isButtonPressed(void)
{
    // S19 버튼이 눌렸는지 확인
    return GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_10) == 0;
}
