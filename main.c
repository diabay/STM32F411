/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LedMode1 1
#define LedMode2 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char rxByte = 0;
uint8_t rxBufferIndex = 0;
char rxBuffer[14] = {0};
const char *txtCfgMode = "CFGMODE";
const char *txtCfgTime = "CFGTIME";
const char *msgCfgTimeOK = "CFGTIME OK\n";
const char *msgCfgTimeFail = "CFGTIME FAIL\n";
const char *msgMode1Select = "MODE1 SELECTED\n";
const char *msgMode2Select = "MODE2 SELECTED\n";
const char *msgCfgModeFail = "CFGMODE FAIL\n";
const char *msgERROR = "ERROR\n";
char msgText[20] = {0};

uint8_t ledMode = 0;
uint32_t tpre = 0;
uint8_t ledNumber = 0;
uint8_t getTimeGreen = 0;
uint8_t getTimeYellow = 0;
uint8_t getTimeRed = 0;
uint8_t getTimeGreenTemp = 0;
uint8_t getTimeYellowTemp = 0;
uint8_t getTimeRedTemp = 0;
uint8_t countTimeGreenOn = 0;
uint8_t countTimeYellowOn = 0;
uint8_t countTimeRedOn = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	__NOP();
	if (ledMode == 1) {
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
	} else if (ledMode == 2) {
		if ((countTimeGreenOn != 0) && (ledNumber == 1)) {
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
			countTimeGreenOn--;
			if (countTimeGreenOn == 0) {
				ledNumber++;
			}
		} else if ((countTimeYellowOn != 0) && (ledNumber == 2)) {
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
			countTimeYellowOn--;
			if (countTimeYellowOn == 0) {
				ledNumber++;
			}
		} else if ((countTimeRedOn != 0) && (ledNumber == 3)) {
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
			countTimeRedOn--;
			if (countTimeRedOn == 0) {
				ledNumber++;
			}
		} else if ((countTimeGreenOn + countTimeYellowOn + countTimeRedOn) == 0) {
			countTimeGreenOn = getTimeGreen;
			countTimeYellowOn = getTimeYellow;
			countTimeRedOn = getTimeRed;
			ledNumber++;
			if (ledNumber > 3) {
				ledNumber = 1;
			}
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (rxByte != '\r') {
		rxBuffer[rxBufferIndex] = rxByte;
		rxBufferIndex++;

		if (rxBufferIndex >= 14) {
			rxBufferIndex = 0;
			memset(msgText, 0, 20);
			strncpy(msgText, msgERROR, strlen(msgERROR));
			HAL_UART_Transmit(&huart2, msgText, strlen(msgText), 1000);
		}
	} else {
		memset(msgText, 0, 20);
		if ((strncmp(rxBuffer, txtCfgMode, strlen(txtCfgMode)) == 0) && (rxBufferIndex == 8)) {
			if (rxBuffer[7] == '1') {
				ledMode = 1;
				strncpy(msgText, msgMode1Select, strlen(msgMode1Select));
			} else if (rxBuffer[7] == '2') {
				ledMode = 2;
				ledNumber = 1;
				countTimeGreenOn = getTimeGreen;
				countTimeYellowOn = getTimeYellow;
				countTimeRedOn = getTimeRed;
				strncpy(msgText, msgMode2Select, strlen(msgMode2Select));
			} else {
				strncpy(msgText, msgCfgModeFail, strlen(msgCfgModeFail));
			}
		} else if ((strncmp(rxBuffer, txtCfgTime, strlen(txtCfgTime)) == 0) && (rxBufferIndex == 13)) {
			getTimeGreenTemp = ((rxBuffer[7] - '0') * 10) + (rxBuffer[8] - '0');
			getTimeYellowTemp = ((rxBuffer[9] - '0') * 10) + (rxBuffer[10] - '0');
			getTimeRedTemp = ((rxBuffer[11] - '0') * 10) + (rxBuffer[12] - '0');
			if (((getTimeGreenTemp <= 15) || (getTimeGreenTemp >= 45)) || ((getTimeYellowTemp <= 3) || (getTimeYellowTemp >= 5)) || ((getTimeRedTemp <= 15) || (getTimeRedTemp >= 90))) {
				strncpy(msgText, msgCfgTimeFail, strlen(msgCfgTimeFail));
			} else {
				strncpy(msgText, msgCfgTimeOK, strlen(msgCfgTimeOK));
				ledMode = 2;
				ledNumber = 1;
				getTimeGreen = getTimeGreenTemp;
				getTimeYellow = getTimeYellowTemp;
				getTimeRed = getTimeRedTemp;
				countTimeGreenOn = getTimeGreen;
				countTimeYellowOn = getTimeYellow;
				countTimeRedOn = getTimeRed;
			}
		} else {
			strncpy(msgText, msgERROR, strlen(msgERROR));
		}
		rxBufferIndex = 0;
		memset(rxBuffer, 0, 14);
		HAL_UART_Transmit(&huart2, msgText, strlen(msgText), 1000);
	}
	HAL_UART_Receive_IT(&huart2, &rxByte, 1);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	memset(msgText, 0, 20);
	if(ledMode == 1){
		ledMode = 2;
		ledNumber = 1;
		countTimeGreenOn = getTimeGreen;
		countTimeYellowOn = getTimeYellow;
		countTimeRedOn = getTimeRed;
		strncpy(msgText, msgMode2Select, strlen(msgMode2Select));
	} else {
		ledMode = 1;
		strncpy(msgText, msgMode1Select, strlen(msgMode1Select));
	}

//	HAL_Delay(50);
//	tpre = HAL_GetTick();
//	while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET){
//		if ((HAL_GetTick() - tpre) > 3000) {
//			break;
//		}
//	}
//	HAL_Delay(50);
	HAL_UART_Transmit(&huart2, msgText, strlen(msgText), 1000);
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

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
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_UART_Receive_IT (&huart2, &rxByte, 1);
  ledNumber = 1;
  ledMode = 1;
  getTimeGreen = 16;
  getTimeYellow = 4;
  getTimeRed = 16;
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV8;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 7999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  huart2.Init.BaudRate = 38400;
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 PD14 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
