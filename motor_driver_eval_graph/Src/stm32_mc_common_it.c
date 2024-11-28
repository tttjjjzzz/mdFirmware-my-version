
/**
  ******************************************************************************
  * @file    stm32_mc_common_it.c
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   Main Interrupt Service Routines.
  *          This file provides exceptions handler and peripherals interrupt
  *          service routine related to Motor Control for the STM32 Family
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  * @ingroup STM32F0xx_IRQ_Handlers
  */

/* Includes ------------------------------------------------------------------*/
#include "mc_config.h"
#include "mc_type.h"
#include "mc_tasks.h"
#include "parameters_conversion.h"
#include "motorcontrol.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx.h"
#include "mcp_config.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/** @addtogroup MCSDK
  * @{
  */
/** @addtogroup STM32F0xx_IRQ_Handlers STM32F0xx IRQ Handlers
  * @{
  */

/* USER CODE BEGIN PRIVATE */
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SYSTICK_DIVIDER (SYS_TICK_FREQUENCY/1000U)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* USER CODE END PRIVATE */

void EXTI4_15_IRQHandler(void);
void HardFault_Handler(void);
void SysTick_Handler(void);

/**
  * @brief  This function handles TIMx global interrupt request for M1 Speed Sensor.
  * @param  None
  */
void SPD_TIM_M1_IRQHandler(void)
{
  /* USER CODE BEGIN SPD_TIM_M1_IRQn 0 */

  /* USER CODE END SPD_TIM_M1_IRQn 0 */

  /* Encoder Timer UPDATE IT is dynamicaly enabled/disabled, checking enable state is required */
  if (LL_TIM_IsEnabledIT_UPDATE(ENCODER_M1.TIMx) != 0U)
  {
    if (LL_TIM_IsActiveFlag_UPDATE(ENCODER_M1.TIMx) != 0U)
    {
      LL_TIM_ClearFlag_UPDATE(ENCODER_M1.TIMx);
      (void)ENC_IRQHandler(&ENCODER_M1);

      /* USER CODE BEGIN M1 ENCODER_Update */

      /* USER CODE END M1 ENCODER_Update */
    }
    else
    {
      /* No other IT to manage for encoder config */
    }
  }
  else
  {
    /* No other IT to manage for encoder config */
  }

  /* USER CODE BEGIN SPD_TIM_M1_IRQn 1 */

  /* USER CODE END SPD_TIM_M1_IRQn 1 */
}

/* This section is present only when MCP over UART_A is used */
/**
  * @brief  This function handles USART interrupt request.
  * @param  None
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQHandler 0 */

  /* USER CODE END USART1_IRQHandler 0 */
  uint32_t flags;
  uint32_t activeIdleFlag;
  uint32_t isEnabledIdleFlag;

  if (0U == LL_USART_IsActiveFlag_TC(USARTA))
  {
    /* Nothing to do */
  }
  else
  {
    /* Disable the DMA channel to prepare the next chunck of data*/
    LL_DMA_DisableChannel(DMA_TX_A, DMACH_TX_A);
    LL_USART_ClearFlag_TC(USARTA);
    /* Data Sent by UART*/
    /* Need to free the buffer, and to check pending transfer*/
    ASPEP_HWDataTransmittedIT(&aspepOverUartA);
  }
  if (LL_USART_IsActiveFlag_ORE(USARTA))
  { /* Stopping the debugger will generate an OverRun error*/
    LL_USART_ClearFlag_ORE(USARTA);
    LL_USART_EnableIT_IDLE(USARTA);
  }
  else
  {
    /* Nothing to do */
  }

  activeIdleFlag = LL_USART_IsActiveFlag_IDLE(USARTA);
  isEnabledIdleFlag = LL_USART_IsEnabledIT_IDLE(USARTA);

  flags = activeIdleFlag & isEnabledIdleFlag;
  if (0U == flags)
  {
    /* Nothing to do */
  }
  else
  { /* Stopping the debugger will generate an OverRun error*/
    LL_USART_DisableIT_IDLE(USARTA);
    /* To be sure we fetch the potential pending data*/
    /* We disable the DMA request, Read the dummy data, endable back the DMA request */
    LL_USART_DisableDMAReq_RX(USARTA);
    (void)LL_USART_ReceiveData8(USARTA);
    LL_USART_EnableDMAReq_RX(USARTA);
    /* Clear pending DMA TC to process only new received packet */
    LL_DMA_ClearFlag_TC(DMA_RX_A, DMACH_RX_A);
    ASPEP_HWReset(&aspepOverUartA);
  }

  /* USER CODE BEGIN USART1_IRQHandler 1 */

  /* USER CODE END USART1_IRQHandler 1 */
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  */
void HardFault_Handler(void)
{
 /* USER CODE BEGIN HardFault_IRQn 0 */

 /* USER CODE END HardFault_IRQn 0 */

  TSK_HardwareFaultTask();

  /* Go to infinite loop when Hard Fault exception occurs */
  while (true)
  {
    /* Nothing to do */
  }

 /* USER CODE BEGIN HardFault_IRQn 1 */

 /* USER CODE END HardFault_IRQn 1 */
}

void SysTick_Handler(void)
{
#ifdef MC_HAL_IS_USED
static uint8_t SystickDividerCounter = SYSTICK_DIVIDER;
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  if (SystickDividerCounter == SYSTICK_DIVIDER)
  {
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
    SystickDividerCounter = 0;
  }
  else
  {
    /* Nothing to do */
  }

  SystickDividerCounter ++;
#endif /* MC_HAL_IS_USED */
  /* Buffer is ready by the HW layer to be processed */
  /* NO DMA interrupt */
  if (LL_DMA_IsActiveFlag_TC(DMA_RX_A, DMACH_RX_A))
  {
    LL_DMA_ClearFlag_TC(DMA_RX_A, DMACH_RX_A);
    ASPEP_HWDataReceivedIT(&aspepOverUartA);
  }
  else
  {
    /* Nothing to do */
  }
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */

    MC_RunMotorControlTasks();

  /* USER CODE BEGIN SysTick_IRQn 2 */

  /* USER CODE END SysTick_IRQn 2 */
}

/**
  * @brief  This function handles Button IRQ on PIN PC15.
  */
void EXTI4_15_IRQHandler(void)
{
  /* USER CODE BEGIN START_STOP_BTN */
  if (LL_EXTI_ReadFlag_0_31(LL_EXTI_LINE_15))
  {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_15);
    (void)UI_HandleStartStopButton_cb();
  }
  else
  {
    /* Nothing to do */
  }

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT 2024 STMicroelectronics *****END OF FILE****/

