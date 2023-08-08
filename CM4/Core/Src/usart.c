/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "debug.h"
#include "main.h"
#include "common.h"
#include "sms_modem.h"
#include "openamp.h"

byte_buffer_t uart3_rx_buf;
byte_buffer_t uart3_tx_buf;
byte_buffer_t uart7_rx_buf;
byte_buffer_t uart7_tx_buf;
/* USER CODE END 0 */

UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;

/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */
	LOG_USART(LEVEL_INFO, "MX_USART3_UART_Init\r\n");
  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

		LOG_USART(LEVEL_INFO, "HAL_UART_MspInit USART3 starts\r\n");
  /* USER CODE END USART3_MspInit 0 */
  if(IS_ENGINEERING_BOOT_MODE())
  {
  /** Initializes the peripherals clock
  */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_UART35;
    PeriphClkInit.Uart35ClockSelection = RCC_UART35CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

  }

    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB12     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART3 DMA Init */
    /* USART3_RX Init */
    hdma_usart3_rx.Instance = DMA1_Stream0;
    hdma_usart3_rx.Init.Request = DMA_REQUEST_USART3_RX;
    hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart3_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart3_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart3_rx);

    /* USART3_TX Init */
    hdma_usart3_tx.Instance = DMA1_Stream1;
    hdma_usart3_tx.Init.Request = DMA_REQUEST_USART3_TX;
    hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_tx.Init.Mode = DMA_NORMAL;
    hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart3_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart3_tx);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

		LOG_USART(LEVEL_INFO, "HAL_UART_MspInit USART3 ends\r\n");
  /* USER CODE END USART3_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB12     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_12);

    /* USART3 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void USART_Start_receive(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART3) {
		LOG_USART(LEVEL_INFO, "3 Start_receive\n\r");

		uart3_rx_buf.ptr = 0;

        __HAL_DMA_DISABLE(huart3.hdmarx);
        __HAL_DMA_SET_COUNTER(huart3.hdmarx, BYTE_BUF_SIZE);
        __HAL_DMA_ENABLE(huart3.hdmarx);

		HAL_UART_Receive_DMA(huart, (uint8_t*) uart3_rx_buf.data, BYTE_BUF_SIZE);
	}
	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART3) {
//		LOG_USART(LEVEL_DEBUG, "3 TxCpltCallback\n\r");
	}
	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);
}

void USART_Send(UART_HandleTypeDef *huart_to, char *buf_from, uint16_t length) {
	char *buf_to = NULL;

	__HAL_UART_DISABLE_IT(huart_to, UART_IT_IDLE);

	if (huart_to->Instance == USART3) {

		buf_to = uart3_tx_buf.data;
		memcpy(buf_to, buf_from, length);

		if (HAL_UART_Transmit(huart_to, (uint8_t*) buf_to, length, 150)
				!= HAL_OK) {
			LOG_USART(LEVEL_ERROR, "Send err\n\r");
		}

		__HAL_UART_ENABLE_IT(huart_to, UART_IT_IDLE);
	}
}

void USART_IDLE_handler(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART3) {
		uart3_rx_buf.ptr = BYTE_BUF_SIZE - huart->hdmarx->Instance->NDTR;
		uart3_rx_buf.data[uart3_rx_buf.ptr] = 0;
		LOG_USART(LEVEL_DEBUG, "Idle\n\r");

		if (modem.flag_wait_responce)
		{
			if ((uart3_rx_buf.ptr > 5 && strstr((char *)&uart3_rx_buf.data[uart3_rx_buf.ptr - 5], "OK\r\n"))
                || (uart3_rx_buf.ptr > 8 && strstr((char *)&uart3_rx_buf.data[uart3_rx_buf.ptr - 8], "ERROR\r\n"))
                || (uart3_rx_buf.ptr > 15 && strnstr((char *)&uart3_rx_buf.data[3], "CMS ERROR", 12)))
			{
				modem.flag_responce_recvd = 1;
				LOG_USART(LEVEL_DEBUG, "Recv: (len %d) %s\n\r", uart3_rx_buf.ptr, uart3_rx_buf.data);
				LOG_USART(LEVEL_DEBUG, "Semaphore released\r\n");
			}
			else
			{
				LOG_USART(LEVEL_DEBUG, "Wait: (len %d) %s\n\r", uart3_rx_buf.ptr, uart3_rx_buf.data);
			}
		}
		else
		{
			uart3_rx_buf.ptr = BYTE_BUF_SIZE - huart->hdmarx->Instance->NDTR;
			LOG_USART(LEVEL_DEBUG, "3 Skip: (len %d) %s\n\r", uart3_rx_buf.ptr, uart3_rx_buf.data);
			memset(uart3_rx_buf.data, 0, uart3_rx_buf.ptr);
			uart3_rx_buf.ptr = 0;

            __HAL_DMA_DISABLE(huart3.hdmarx);
            __HAL_DMA_SET_COUNTER(huart3.hdmarx, BYTE_BUF_SIZE);
            __HAL_DMA_ENABLE(huart3.hdmarx);
		}
	}
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
