#include "main.h"

#include "cBot/cBot.h"
#include "cBot/cBotApp.h"
// #include "marioSong.h"

// ADC_HandleTypeDef hadc1;
// DMA_HandleTypeDef hdma_adc1;
//
// I2C_HandleTypeDef hi2c1;
//
// TIM_HandleTypeDef htim1;
// TIM_HandleTypeDef htim2;
// TIM_HandleTypeDef htim3;
// TIM_HandleTypeDef htim4;
// DMA_HandleTypeDef hdma_tim2_ch1;
//
// void SystemClock_Config(void);
// static void MX_GPIO_Init(void);
// static void MX_DMA_Init(void);
// static void MX_ADC1_Init(void);
// static void MX_I2C1_Init(void);
// static void MX_TIM1_Init(void);
// static void MX_TIM2_Init(void);
// static void MX_TIM3_Init(void);
// static void MX_USART1_UART_Init(void);
// static void MX_TIM4_Init(void);

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
  // SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  // MX_GPIO_Init();
  // MX_DMA_Init();
  // MX_ADC1_Init();
  // MX_I2C1_Init();
  // MX_TIM1_Init();
  // MX_TIM2_Init();
  // MX_TIM3_Init();
  // MX_USART1_UART_Init();
  // MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  cBot_init();
  init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


  while (1)
  {
	  loop();
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

void SysTick_Handler(void)
{
  HAL_IncTick();
}

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
  while (1) {}
}


void MemManage_Handler(void)
{
  while (1) {}
}

void BusFault_Handler(void)
{
  while (1) {}
}

void UsageFault_Handler(void)
{
  while (1) {}
}

void SVC_Handler(void)
{
}


void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}
