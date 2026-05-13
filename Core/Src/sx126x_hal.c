/*
 * sx126x_hal.c
 *
 *  Created on: May 7, 2026
 *      Author: HP
 */


#include "sx126x_hal.h"
#include "main.h"

extern SPI_HandleTypeDef hspi3;

/*
 * IMPORTANT:
 * Replace these with YOUR actual pins
 */

#define SX_NSS_PORT    GPIOD
#define SX_NSS_PIN     GPIO_PIN_14

#define SX_BUSY_PORT   GPIOC
#define SX_BUSY_PIN    GPIO_PIN_8

#define SX_RESET_PORT  GPIOD
#define SX_RESET_PIN   GPIO_PIN_15

/*
 * --------------------------------------------------------------------------
 * WAIT BUSY
 * --------------------------------------------------------------------------
 */

static void sx126x_wait_on_busy(void)
{
    while(HAL_GPIO_ReadPin(SX_BUSY_PORT, SX_BUSY_PIN) == GPIO_PIN_SET);
}

/*
 * --------------------------------------------------------------------------
 * WRITE
 * --------------------------------------------------------------------------
 */

sx126x_hal_status_t sx126x_hal_write(
    const void* context,
    const uint8_t* command,
    const uint16_t command_length,
    const uint8_t* data,
    const uint16_t data_length )
{
    sx126x_wait_on_busy();

    HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_RESET);

    if(HAL_SPI_Transmit(&hspi3, (uint8_t*)command, command_length, HAL_MAX_DELAY) != HAL_OK)
    {
        HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_SET);
        return SX126X_HAL_STATUS_ERROR;
    }

    if(data_length > 0)
    {
        if(HAL_SPI_Transmit(&hspi3, (uint8_t*)data, data_length, HAL_MAX_DELAY) != HAL_OK)
        {
            HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_SET);
            return SX126X_HAL_STATUS_ERROR;
        }
    }

    HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_SET);

    sx126x_wait_on_busy();

    return SX126X_HAL_STATUS_OK;
}

/*
 * --------------------------------------------------------------------------
 * READ
 * --------------------------------------------------------------------------
 */

sx126x_hal_status_t sx126x_hal_read(
    const void* context,
    const uint8_t* command,
    const uint16_t command_length,
    uint8_t* data,
    const uint16_t data_length )
{
    sx126x_wait_on_busy();

    HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_RESET);

    if(HAL_SPI_Transmit(&hspi3, (uint8_t*)command, command_length, HAL_MAX_DELAY) != HAL_OK)
    {
        HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_SET);
        return SX126X_HAL_STATUS_ERROR;
    }

    if(HAL_SPI_Receive(&hspi3, data, data_length, HAL_MAX_DELAY) != HAL_OK)
    {
        HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_SET);
        return SX126X_HAL_STATUS_ERROR;
    }

    HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_SET);

    sx126x_wait_on_busy();

    return SX126X_HAL_STATUS_OK;
}

/*
 * --------------------------------------------------------------------------
 * RESET
 * --------------------------------------------------------------------------
 */

sx126x_hal_status_t sx126x_hal_reset(const void* context)
{
    HAL_GPIO_WritePin(SX_RESET_PORT, SX_RESET_PIN, GPIO_PIN_RESET);

    HAL_Delay(20);

    HAL_GPIO_WritePin(SX_RESET_PORT, SX_RESET_PIN, GPIO_PIN_SET);

    HAL_Delay(20);

    sx126x_wait_on_busy();

    return SX126X_HAL_STATUS_OK;
}

/*
 * --------------------------------------------------------------------------
 * WAKEUP
 * --------------------------------------------------------------------------
 */

sx126x_hal_status_t sx126x_hal_wakeup(const void* context)
{
    sx126x_wait_on_busy();

    HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_RESET);

    uint8_t cmd = 0xC0;

    HAL_SPI_Transmit(&hspi3, &cmd, 1, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(SX_NSS_PORT, SX_NSS_PIN, GPIO_PIN_SET);

    HAL_Delay(1);

    sx126x_wait_on_busy();

    return SX126X_HAL_STATUS_OK;
}
