/**
  ******************************************************************************
  * @file    Audio/Audio_playback_and_record/Src/main.c 
  * @author  MCD Application Team
  * @version V1.2.2
  * @date    25-May-2015
  * @brief   Main program body.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
#include "application.h"
#include "utils.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef hTimLed;
TIM_OC_InitTypeDef sConfigLed;

/* Counter for User button presses. Defined as external in waveplayer.c file */
__IO uint32_t PressCount = 1;

/* Capture Compare Register Value.
   Defined as external in stm32f4xx_it.c file */
__IO uint16_t CCR1Val = 4800;              

__IO uint32_t PbPressCheck = 0;

USBH_HandleTypeDef hUSBHost; /* USB Host handle */

/* LED State (Toggle or OFF)*/
__IO uint32_t LEDsState;

/* Private function prototypes -----------------------------------------------*/
static void TIM_LED_Config(void);
static void SystemClock_Config(void);
static void USBH_UserProcess(USBH_HandleTypeDef *pHost, uint8_t vId);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* STM32F4xx HAL library initialization:
      - Configure the Flash prefetch, instruction and Data caches
      - Configure the Systick to generate an interrupt each 1 msec
      - Set NVIC Group Priority to 4
      - Global MSP (MCU Support Package) initialization
  */
  HAL_Init();
  
  /* Configure LED3, LED4, LED5 and LED6 */
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
  BSP_LED_Init(LED5);
  BSP_LED_Init(LED6);
 
  /* Configure the system clock to 84 MHz */
  SystemClock_Config(); 
   
  /* Initialize MEMS Accelerometer mounted on STM32F4-Discovery board */
  if(BSP_ACCELERO_Init() != ACCELERO_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
  /* Enable click config for pause/play */
  BSP_ACCELERO_Click_ITConfig();
  
  /* Turn ON LED4: start of application */
  //BSP_LED_On(LED4);
  
  /* Configure TIM4 Peripheral to manage LEDs lighting */
  TIM_LED_Config();
  
  /* Turn OFF all LEDs */
  LEDsState = LEDS_OFF;
  
  /* Initialize User Button */
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
  
  /*##-1- Link the USB Host disk I/O driver ##################################*/
  application_init();
  
  /*##-2- Init Host Library ################################################*/
  USBH_Init(&hUSBHost, USBH_UserProcess, 0);
  
  /*##-3- Add Supported Class ##############################################*/
  USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);
  
  /*##-4- Start Host Process ###############################################*/
  USBH_Start(&hUSBHost);  
  
  /* Run Application (Blocking mode)*/
  while (1)
  {
    application_task();
    
    /* USBH_Background Process */
    USBH_Process(&hUSBHost);
  }
}

/**
  * @brief  User Process
  * @param  phost: Host Handle
  * @param  id: Host Library user message ID
  * @retval None
  */
static void USBH_UserProcess (USBH_HandleTypeDef *pHost, uint8_t vId)
{  
  switch (vId)
  { 
  case HOST_USER_SELECT_CONFIGURATION:
    break;
    
  case HOST_USER_DISCONNECTION:
    application_disconect();
    break;
    
  case HOST_USER_CLASS_ACTIVE:
    application_conect();
    break;
      
  default:
    break;
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 84000000
  *            HCLK(Hz)                       = 84000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 4
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale2 mode
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
 
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  Configures TIM4 Peripheral for LEDs lighting.
  * @param  None
  * @retval None
  */
static void TIM_LED_Config(void)
{
  uint16_t prescalervalue = 0;
  uint32_t tmpvalue = 0;

  /* TIM4 clock enable */
  __HAL_RCC_TIM4_CLK_ENABLE();
  
  /* Enable the TIM4 global Interrupt */
  HAL_NVIC_SetPriority(TIM4_IRQn, 7, 0);  
  HAL_NVIC_EnableIRQ(TIM4_IRQn);
  
  /* -----------------------------------------------------------------------
  TIM4 Configuration: Output Compare Timing Mode:  
    To get TIM4 counter clock at 550 KHz, the prescaler is computed as follows:
    Prescaler = (TIM4CLK / TIM4 counter clock) - 1
    Prescaler = ((f(APB1) * 2) /550 KHz) - 1
  
    CC update rate = TIM4 counter clock / CCR_Val = 32.687 Hz
    ==> Toggling frequency = 16.343 Hz  
  ----------------------------------------------------------------------- */
  
  /* Compute the prescaler value */
  tmpvalue = HAL_RCC_GetPCLK1Freq();
  prescalervalue = (uint16_t) ((tmpvalue * 2) / 480000) - 1; //Configurado a 48 KHz
  
  /* Time base configuration */
  hTimLed.Instance = TIM4;
  hTimLed.Init.Period = 65535;
  hTimLed.Init.Prescaler = prescalervalue;
  hTimLed.Init.ClockDivision = 0;
  hTimLed.Init.CounterMode = TIM_COUNTERMODE_UP;
  if(HAL_TIM_OC_Init(&hTimLed) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }
  
  /* Output Compare Timing Mode configuration: Channel1 */
  sConfigLed.OCMode = TIM_OCMODE_TIMING;
  sConfigLed.OCIdleState = TIM_OCIDLESTATE_SET;
  sConfigLed.Pulse = CCR1Val;
  sConfigLed.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigLed.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigLed.OCFastMode = TIM_OCFAST_ENABLE;
  sConfigLed.OCNIdleState = TIM_OCNIDLESTATE_SET;
  
  /* Initialize the TIM4 Channel1 with the structure above */
  if(HAL_TIM_OC_ConfigChannel(&hTimLed, &sConfigLed, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler();
  }

  /* Start the Output Compare */
  if(HAL_TIM_OC_Start_IT(&hTimLed, TIM_CHANNEL_1) != HAL_OK)
  {
    /* Start Error */
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* Turn LED3 on */
  BSP_LED_On(LED3);
  while(1)
  {
  }
}

/**
  * @brief  Output Compare callback in non blocking mode 
  * @param  htim: TIM OC handle
  * @retval None
  */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  uint32_t capture = 0; 

  /* Get the TIM4 Input Capture 1 value */
  capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
  
  /* Set the TIM4 Capture Compare1 Register value */
  __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, (CCR1Val + capture));
}



 /**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == GPIO_PIN_0) 
  {
    if(PbPressCheck == 0)
    {
      PbPressCheck = 1;
    }
    else
    {
      PbPressCheck = 0;
    }
  }
  
  if(GPIO_Pin == ACCELERO_INT1_PIN) 
  {
    if(PressCount == 1)
    {
      PressCount = 0;
    }
    else
    {
      PressCount = 1;
    }
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
