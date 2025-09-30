/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "clcd.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
#define ARR_CNT 5
#define CMD_SIZE 50
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
volatile unsigned int time3Cnt = 0;
volatile int time3CntFlag = 1;
volatile unsigned int time3SecCnt = 0;
volatile int time3SecFlag = 0;
volatile int alarmOn = 0;

//volatile int timerIsrFlag = 0;    //아마 1초? time3SecFlag 로 대신해도?
//unsigned int secCount = 0;
//volatile int updatTimeFlag = 0;

typedef struct {
  int hour;
  int min;
  int sec;
  int msec;
  char apm[3]; //'A' : AM, 'P' : PM
} ATIME;
ATIME Time = { 12, 0, 0, 0, "AM"};
ATIME ARTime = { 12, 0, 0, 0, "AM"};
char buff[30];
int alarmCnt = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
int getKeyNum();

void clock_display();
void clock_calc(ATIME *Time);
void clock_set();
void alarm_buzzer();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	int cnt = 0;
	int keyNum = 0;
	int mode = 0;
	ATIME *targetTime;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  printf("main start()!!\r\n");


  if(HAL_TIM_Base_Start_IT(&htim3) != HAL_OK)
	  Error_Handler();
  LCD_init(&hi2c1);
//  LCD_writeStringXY(0, 0, "hello lcd");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(time3SecFlag)
	  {
		  HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
		  printf("time3SecCnt : %d \r\n", time3SecCnt);
		  clock_display();

		  if(alarmOn)
		  {
		        alarm_buzzer();  // 토글
		        alarmCnt++;
		        if (alarmCnt >= 60)
		        {
		            alarmOn = 0;
		            alarmCnt = 0;
		            HAL_GPIO_WritePin(BUZZER_GPIO_Port , BUZZER_Pin, GPIO_PIN_RESET);  // 끄기
		        }
		  }
		  if (!alarmOn && memcmp(&Time, &ARTime, sizeof(ATIME)) == 0)
		  {
		      alarmOn = 1;
		      alarmCnt = 0;
		  }

		  clock_calc(&Time);
		  time3SecFlag = 0;
	  }

#if 0
	  if (!alarmOn && memcmp(&Time, &ARTime, sizeof(ATIME)) == 0)
	  {
		  alarmOn = 1;
		  alarmCnt = 0;

		  alarm_buzzer();
	  }

	  if(alarmCnt > 60)
	  {
		  alarmCnt = 0;
		  HAL_GPIO_WritePin(BUZZER_GPIO_Port , BUZZER_Pin, GPIO_PIN_RESET);
	  }
#endif


	  if(mode == 0)
		  targetTime = 0;
	  if(mode == 1)
		  targetTime = &ARTime;
	  if(mode == 2)
		  targetTime = &Time;

	  keyNum = getKeyNum();
	  if(keyNum != 0)
	  {
		  //printf("keyNum : %d \r\n", keyNum);
		  if (keyNum == 1)  //시간 설정
		  {
			  targetTime->hour++;
			  if(targetTime->hour > 12)
			  {
				  targetTime->hour = 1;
			  }
			  if(targetTime->hour == 12 && targetTime->min == 0 && targetTime->sec == 0)
			  {
		          if (strcmp(targetTime->apm, "AM") == 0)
		              strcpy(targetTime->apm, "PM");
		          else
		              strcpy(targetTime->apm, "AM");
			  }
		  }
		  if (keyNum == 2)  //분 설정
		  {
			  //printf("keyNum : %d \r\n", keyNum);
			  targetTime->min++;
			  if(targetTime->min >= 60)
			  {
				  targetTime->min = 0;
				  targetTime->hour++;
				  if(targetTime->hour > 12)
				  {
					  targetTime->hour = 1;
				  }
				  if(targetTime->hour == 12 && targetTime->min == 0 && targetTime->sec == 0)
				  {
			          if (strcmp(targetTime->apm, "AM") == 0)
			              strcpy(targetTime->apm, "PM");
			          else
			              strcpy(targetTime->apm, "AM");
				  }
			  }
		  }
		  if (keyNum == 3)  //초 설정
		  {
			  targetTime->sec++;
			  if(targetTime->sec >= 60)
			  {
				  targetTime->sec = 0;
				  targetTime->min++;
				  if(targetTime->min >= 60)
				  {
					  targetTime->min = 0;
					  targetTime->hour++;
					  if(targetTime->hour > 12)
					  {
						  targetTime->hour = 1;
					  }
					  if(targetTime->hour == 12 && targetTime->min == 0 && targetTime->sec == 0)
					  {
				          if (strcmp(targetTime->apm, "AM") == 0)
				              strcpy(targetTime->apm, "PM");
				          else
				              strcpy(targetTime->apm, "AM");
					  }
				  }
			  }
		  }
		  if (keyNum == 4)  //시간/알람 설정 mode
		  {
			  printf("keyNum : %d , cnt : %d \r\n", keyNum, ++cnt);
			  if(cnt % 3 == 1)
			  {
				  LCD_writeStringXY(0, 14, " ");
				  LCD_writeStringXY(1, 14, "*");
				  targetTime = &ARTime;
				  mode = 1;
				  alarmOn = 0;
			  }
			  if(cnt % 3 == 2)
			  {
				  LCD_writeStringXY(0, 14, "*");
				  LCD_writeStringXY(1, 14, " ");
				  targetTime = &Time;
				  mode = 2;
				  alarmOn = 0;
			  }
			  if(cnt % 3 == 0)
			  {
				  LCD_writeStringXY(0, 14, " ");
				  LCD_writeStringXY(1, 14, " ");
				  mode = 0;

			  }
		  }
	  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 8400-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, TEST_LED_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN0_Pin BTN1_Pin BTN2_Pin BTN3_Pin */
  GPIO_InitStruct.Pin = BTN0_Pin|BTN1_Pin|BTN2_Pin|BTN3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : TEST_LED_Pin LD2_Pin */
  GPIO_InitStruct.Pin = TEST_LED_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BUZZER_Pin */
  GPIO_InitStruct.Pin = BUZZER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BUZZER_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART6 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

  return ch;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim)
{
	//static unsigned long time3Cnt = 0;
	time3Cnt++;			  //100ms
	time3CntFlag = 1;
	if(time3Cnt >= 10)  //100ms * 10 = 1sec
	{
		time3SecFlag = 1;
		time3SecCnt++;
		time3Cnt = 0;		//100ms
	}
}

int getKeyNum()
{
	int keyNum = 0;
	int buttonState = 0;
	static int keyNumOld = 0;
	for (int i=0; i<4; i++)
	{
		  buttonState = HAL_GPIO_ReadPin(BTN0_GPIO_Port, BTN0_Pin<<i);
		  if(buttonState)
		  {
			  keyNum = i+1;
			  if(keyNum != keyNumOld)
			  {
				  keyNumOld = keyNum;
				  return keyNum;
			  }
			  else
				  return 0;
		  }
	}
	keyNumOld = 0;
	return keyNum;
}

/*
 void timerIsr() {
  timerIsrFlag = 1;
  secCount++;
  clock_calc(&Time);
}
*/

void clock_display()	//시계/알람 LCD 출력 함수
{
#if 1
//	  LCD_writeStringXY(0, 0, "hello lcd");

//	  dht11Data = DHT11_readData();
	  sprintf(buff,"T %02d:%02d:%02d %s ", Time.hour, Time.min, Time.sec, Time.apm);
	  printf("%s\r\n", buff);
	  LCD_writeStringXY(0, 0, buff);

	  sprintf(buff,"A %02d:%02d:%02d %s ", ARTime.hour, ARTime.min, ARTime.sec, ARTime.apm);
	  printf("%s\r\n", buff);
	  LCD_writeStringXY(1, 0, buff);
	  //HAL_Delay(2000);
#endif
}

void clock_calc(ATIME *Time)	//시간 계산 함수
{
  Time->sec++;  // increment second

  if (Time->sec >= 60)  // if second = 60, second = 0
  {
    Time->sec = 0;
    Time->min++;

    if (Time->min >= 60)  // if minute = 60, minute = 0
    {
      Time->min = 0;
      Time->hour++;  // increment hour

      if (Time->hour > 12)
      {
        Time->hour = 1;
      }

      if(Time->hour == 12 && Time->min == 0 && Time->sec ==0)
      {
          if (strcmp(Time->apm, "AM") == 0)
              strcpy(Time->apm, "PM");
          else
              strcpy(Time->apm, "AM");
      }
    }
  }
}

void clock_set()	//시간/알람 설정 함수
{

}
void alarm_buzzer() //LED 부저 출력
{
	HAL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin);
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
