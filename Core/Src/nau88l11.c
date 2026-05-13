
#include "nau88l11.h"

extern I2C_HandleTypeDef hi2c1;
extern I2S_HandleTypeDef hi2s2;

#define NAU88L11_ADDR    (0x1B << 1)

int16_t nau_audio_rx[256];

/* ============================================================
   DEBUG VARIABLES
============================================================ */

volatile HAL_StatusTypeDef dbg_ret;
volatile uint32_t dbg_i2c_err;
volatile uint8_t dbg_failed_reg;
volatile uint16_t dbg_failed_val;
volatile int16_t dbg_sample;

/* ============================================================
   LOW LEVEL REGISTER WRITE
============================================================ */

 HAL_StatusTypeDef nau_write(uint8_t reg,
                                   uint16_t value)
{
    uint8_t data[2];

    /*
        NAU88L11 register format
    */

    data[0] = ((reg & 0x7F) << 1)
            | ((value >> 8) & 0x01);

    data[1] = value & 0xFF;

    dbg_ret = HAL_I2C_Master_Transmit(&hi2c1,
                                      NAU88L11_ADDR,
                                      data,
                                      2,
                                      1000);

    dbg_i2c_err = hi2c1.ErrorCode;

    if(dbg_ret != HAL_OK)
    {
        dbg_failed_reg = reg;
        dbg_failed_val = value;

        /*
            BREAKPOINT HERE
        */
        __NOP();
    }

    return dbg_ret;
}

/* ============================================================
   SIMPLE PING
============================================================ */

HAL_StatusTypeDef NAU88L11_Ping(void)
{
    dbg_ret = HAL_I2C_IsDeviceReady(&hi2c1,
                                    NAU88L11_ADDR,
                                    5,
                                    1000);

    dbg_i2c_err = hi2c1.ErrorCode;

    volatile uint32_t instance_addr = (uint32_t)hi2c1.Instance;
    volatile uint32_t expected      = 0x40005400;  // I2C1 base
    __NOP(); // breakpoint

    return dbg_ret;
}

HAL_StatusTypeDef nau_read(uint8_t reg, uint16_t *value)
{
    uint8_t rx[2] = {0};
    uint8_t reg_addr = reg & 0x7F;   // no shift, just 7-bit reg address

    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(
        &hi2c1,
        NAU88L11_ADDR,          // device addr
        reg_addr,               // register address
        I2C_MEMADD_SIZE_8BIT,   // 1-byte reg addr
        rx,                     // data out
        2,                      // 2 bytes back
        1000
    );

    if (ret != HAL_OK) return ret;

    *value = ((rx[0] & 0x01) << 8) | rx[1];
    return HAL_OK;
}
/* ============================================================
   CODEC INIT
============================================================ */
HAL_StatusTypeDef NAU88L11_InitMicPath(void)
{
    HAL_StatusTypeDef ret;

    /* Software reset */
    ret = nau_write(0x00, 0x0000);
    if(ret != HAL_OK) return ret;
    HAL_Delay(100);

    /* ADC soft mute */
    ret = nau_write(0x31, 0x0200);
    if(ret != HAL_OK) return ret;

    /* DAC ctrl - CIC_GAIN_ADJ=7, OSR128 */
    ret = nau_write(0x2C, 0x0072);
    if(ret != HAL_OK) return ret;

    /* VMID enable 125k */
    ret = nau_write(0x66, 0x0060);
    if(ret != HAL_OK) return ret;
    HAL_Delay(500);

    /* THD boost */
    ret = nau_write(0x69, 0x0020);
    if(ret != HAL_OK) return ret;

    /* ADC analog power up */
    ret = nau_write(0x72, 0x0140);
    if(ret != HAL_OK) return ret;

    /* DAC analog enable + clock */
    ret = nau_write(0x73, 0x1108);
    if(ret != HAL_OK) return ret;

    /* Global bias enable, VMID precharge disable */
    ret = nau_write(0x76, 0x3040);
    if(ret != HAL_OK) return ret;
    HAL_Delay(200);

    /* MICBIAS power up, low noise mode */
    uint8_t test =0;
    ret = nau_write(0x74, 0x0104);
    if(ret != HAL_OK) return ret;

    HAL_Delay(10);

    nau_read(0x74, &test);
    __NOP();

    /* Digital ADC+DAC enable, all clocks */
    ret = nau_write(0x01, 0x3FC2);
    if(ret != HAL_OK) return ret;

    /* Clock divider - ADC/DAC src = CODEC/2 */
    ret = nau_write(0x03, 0x0000);
    if(ret != HAL_OK) return ret;

    /* I2S format, 16-bit, slave */
    ret = nau_write(0x1C, 0x0002);
    if(ret != HAL_OK) return ret;

    /* ← FIX: SMPL_RATE=001(32k closest to 16k), OSR=10(128) */
    ret = nau_write(0x2B, 0x0022);
    if(ret != HAL_OK) return ret;

    /* ← ADD: FEPGA - differential mic input mode */
    ret = nau_write(0x77, 0x0000);
    if(ret != HAL_OK) return ret;

    /* ← ADD: FEPGA_ACDC_CTRL - connect MIC+/MIC- to PGA */
    ret = nau_write(0x76, 0x3043);
    if(ret != HAL_OK) return ret;

    /* PGA gain 10dB */
    ret = nau_write(0x7E, 0x0B00);
    if(ret != HAL_OK) return ret;

    /* PGA power up */
    ret = nau_write(0x7F, 0x8000);
    if(ret != HAL_OK) return ret;

    /* ADC unmute */
    ret = nau_write(0x31, 0x0000);
    if(ret != HAL_OK) return ret;

    HAL_Delay(100);
    return HAL_OK;
}


HAL_StatusTypeDef NAU88L11_MinimalTest(void)
{
    HAL_StatusTypeDef ret;

    /*
    ============================================
    SOFTWARE RESET
    ============================================
    */
    ret = nau_write(0x00, 0x0000);
    if(ret != HAL_OK)
        return ret;

    HAL_Delay(100);

    /*
    ============================================
    VMID ENABLE
    VMID = 125k
    ============================================
    */
    ret = nau_write(0x66, 0x0060);
    if(ret != HAL_OK)
        return ret;

    HAL_Delay(300);

    /*
    ============================================
    GLOBAL ANALOG BIAS ENABLE
    ============================================
    */
    ret = nau_write(0x76, 0x3040);
    if(ret != HAL_OK)
        return ret;

    HAL_Delay(300);

    /*
    ============================================
    MICBIAS ENABLE
    ============================================
    */
    ret = nau_write(0x74, 0x0104);
    if(ret != HAL_OK)
        return ret;

    HAL_Delay(500);

    /*
    ============================================
    DEBUG BREAKPOINT
    ============================================
    */
    __NOP();

    return HAL_OK;
}
/* ============================================================
   START DMA AUDIO RX
============================================================ */

HAL_StatusTypeDef NAU88L11_StartRxDMA(void)
{
    dbg_ret = HAL_I2S_Receive_DMA(&hi2s2,
                                  (uint16_t*)nau_audio_rx,
                                  256);

    /*
        BREAKPOINT HERE
    */
    __NOP();

    return dbg_ret;
}

/* ============================================================
   DMA HALF CALLBACK
============================================================ */

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    dbg_sample = nau_audio_rx[0];

    volatile int16_t s0 = nau_audio_rx[0];
    volatile int16_t s1 = nau_audio_rx[1];
    volatile int16_t s2 = nau_audio_rx[2];
    volatile int16_t s3 = nau_audio_rx[3];
    volatile int16_t s4 = nau_audio_rx[4];
    volatile int16_t s5 = nau_audio_rx[5];
    volatile int16_t s6 = nau_audio_rx[6];
    volatile int16_t s7 = nau_audio_rx[7];



    /*
        BREAKPOINT HERE

        Speak into mic.

        Watch:
        dbg_sample

        GOOD:
            values continuously changing

        BAD:
            always 0
            fixed value
            random huge garbage
    */

    __NOP();
}


