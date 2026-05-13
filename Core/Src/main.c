/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include"sx126x.h"
#include "sx126x_hal.h"
#include "sx126x_regs.h"
#include "nau88l11.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

I2S_HandleTypeDef hi2s2;
DMA_HandleTypeDef hdma_spi2_rx;

SPI_HandleTypeDef hspi3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI3_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t tx_data[] = "HELLL";

void SX1261_Init(void)
{
    // --------------------------------------------------
    // RESET
    // --------------------------------------------------

    sx126x_hal_reset(NULL);

    HAL_Delay(20);

    // --------------------------------------------------
    // TCXO via DIO3
    // --------------------------------------------------

    sx126x_set_dio3_as_tcxo_ctrl(
        NULL,
        SX126X_TCXO_CTRL_3_3V,
        5000
    );

    HAL_Delay(50);

    // --------------------------------------------------
    // STANDBY XOSC
    // --------------------------------------------------

    sx126x_set_standby(
        NULL,
        SX126X_STANDBY_CFG_XOSC
    );

    HAL_Delay(10);

    // --------------------------------------------------
    // DIO2 RF SWITCH
    // --------------------------------------------------

//    sx126x_set_dio2_as_rf_sw_ctrl(
//        NULL,
//        true
//    );

    // --------------------------------------------------
    // PACKET TYPE = GFSK
    // --------------------------------------------------

    sx126x_set_pkt_type(
        NULL,
        SX126X_PKT_TYPE_GFSK
    );

    // --------------------------------------------------
    // RF FREQUENCY
    // --------------------------------------------------

    sx126x_set_rf_freq(
        NULL,
        433000000
    );

    // --------------------------------------------------
    // IMAGE CALIBRATION
    // --------------------------------------------------

    sx126x_cal_img(
        NULL,
        0x6B,
        0x6F
    );

    // --------------------------------------------------
    // PA CONFIG
    // --------------------------------------------------

    sx126x_pa_cfg_params_t pa_cfg;

    pa_cfg.pa_duty_cycle = 0x04;
    pa_cfg.hp_max        = 0x07;
    pa_cfg.device_sel    = 0x00;
    pa_cfg.pa_lut        = 0x01;

    sx126x_set_pa_cfg(
        NULL,
        &pa_cfg
    );

    // --------------------------------------------------
    // TX POWER
    // --------------------------------------------------

    sx126x_set_tx_params(
        NULL,
        10,
        SX126X_RAMP_200_US
    );

    // --------------------------------------------------
    // GFSK MODULATION PARAMS
    // --------------------------------------------------

    sx126x_mod_params_gfsk_t mod_params;

    mod_params.br_in_bps =
        50000;

    mod_params.fdev_in_hz =
        25000;

    mod_params.pulse_shape =
        SX126X_GFSK_PULSE_SHAPE_BT_1;

    mod_params.bw_dsb_param =
        SX126X_GFSK_BW_117300;

    sx126x_set_gfsk_mod_params(
        NULL,
        &mod_params
    );

    // --------------------------------------------------
    // GFSK PACKET PARAMS
    // --------------------------------------------------

    sx126x_pkt_params_gfsk_t pkt_params;

    pkt_params.preamble_len_in_bits =
        32;

    pkt_params.preamble_detector =
        SX126X_GFSK_PREAMBLE_DETECTOR_MIN_16BITS;

    pkt_params.sync_word_len_in_bits =
        32;

    pkt_params.address_filtering =
        SX126X_GFSK_ADDRESS_FILTERING_DISABLE;

    pkt_params.header_type =
        SX126X_GFSK_PKT_VAR_LEN;

    pkt_params.pld_len_in_bytes =
        5;

    pkt_params.crc_type =
        SX126X_GFSK_CRC_2_BYTES;

    pkt_params.dc_free =
        SX126X_GFSK_DC_FREE_OFF;

    sx126x_set_gfsk_pkt_params(
        NULL,
        &pkt_params
    );

    // --------------------------------------------------
    // GFSK SYNC WORD
    // --------------------------------------------------

    uint8_t sync_word[4] =
    {
        0x12,
        0x34,
        0x56,
        0x78
    };

    sx126x_set_gfsk_sync_word(
        NULL,
        sync_word,
        4
    );

    // --------------------------------------------------
    // BUFFER BASE
    // --------------------------------------------------

    sx126x_set_buffer_base_address(
        NULL,
        0x00,
        0x00
    );

    // --------------------------------------------------
    // IRQ CONFIG
    // --------------------------------------------------

    sx126x_set_dio_irq_params(
        NULL,

        SX126X_IRQ_TX_DONE |
        SX126X_IRQ_RX_DONE |
        SX126X_IRQ_TIMEOUT |
        SX126X_IRQ_CRC_ERROR,

        SX126X_IRQ_TX_DONE |
        SX126X_IRQ_RX_DONE |
        SX126X_IRQ_TIMEOUT |
        SX126X_IRQ_CRC_ERROR,

        SX126X_IRQ_NONE,
        SX126X_IRQ_NONE
    );

    // --------------------------------------------------
    // KEEP XOSC ACTIVE
    // --------------------------------------------------

    sx126x_set_rx_tx_fallback_mode(
        NULL,
        SX126X_FALLBACK_STDBY_XOSC
    );

    // --------------------------------------------------
    // CLEAR ERRORS
    // --------------------------------------------------

    sx126x_clear_device_errors(NULL);
}


void SX1261_Send(uint8_t* data, uint8_t len)
{
    // Clear old IRQs
    sx126x_clear_irq_status(
        NULL,
        SX126X_IRQ_ALL
    );

    // Write payload
    sx126x_write_buffer(
        NULL,
        0x00,
        data,
        len
    );

    // Start TX
    sx126x_set_tx(
        NULL,
        5000
    );
    __NOP();

}
volatile uint32_t debug_stage = 0;

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
  MX_DMA_Init();
  MX_SPI3_Init();
  MX_I2C1_Init();
  MX_I2S2_Init();
  /* USER CODE BEGIN 2 */

//  SX1261_Init();
//
//  HAL_Delay(100);

  if (NAU88L11_Ping() != HAL_OK)
      Error_Handler();

  if (NAU88L11_InitMicPath() != HAL_OK)
      Error_Handler();

  if (NAU88L11_StartRxDMA() != HAL_OK)
      Error_Handler();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	    nau_write(0x74, 0x0104);

	    HAL_Delay(1300);

	    nau_write(0x74, 0x0000);

	    HAL_Delay(1300);

//	  // Clear IRQs
//	      sx126x_clear_irq_status(
//	          NULL,
//	          SX126X_IRQ_ALL
//	      );
//
//	      // Send packet
//	      SX1261_Send(tx_data, 5);
//
//	      // Wait TX complete
//	      HAL_Delay(1500);
//
//	      // Read IRQ
//	      sx126x_irq_mask_t irq;
//
//	      sx126x_get_irq_status(
//	          NULL,
//	          &irq
//	      );
//
//	      // Read status
//	      sx126x_chip_status_t status;
//
//	      sx126x_get_status(
//	          NULL,
//	          &status
//	      );
//
//	      // Read errors
//	      sx126x_errors_mask_t errors;
//
//	      sx126x_get_device_errors(
//	          NULL,
//	          &errors
//	      );
//	      HAL_Delay(100);
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
  RCC_OscInitStruct.PLL.PLLN = 192;
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
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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
  * @brief I2S2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S2_Init(void)
{

  /* USER CODE BEGIN I2S2_Init 0 */

  /* USER CODE END I2S2_Init 0 */

  /* USER CODE BEGIN I2S2_Init 1 */

  /* USER CODE END I2S2_Init 1 */
  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_RX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_16K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  hi2s2.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S2_Init 2 */

  /* USER CODE END I2S2_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_SET);

  /*Configure GPIO pins : PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PC7 PC8 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

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
