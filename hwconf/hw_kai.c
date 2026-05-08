/*
 * hw_kai.c
 *
 * Implementasi fungsi hardware untuk KAI INVERTER
 * Pasangan dari hw_kai.h
 *
 * Fungsi utama yang WAJIB ada:
 *   hw_init_gpio()      -> setup semua pin GPIO
 *   hw_setup_adc_channels() -> mapping ADC channel
 */

#include "hw.h"
#include "hw_kai.h"
#include "ch.h"
#include "hal.h"
#include "stm32f4xx_conf.h"
#include "utils.h"
#include "mcpwm_foc.h"    
#include "mc_interface.h"  
#include <math.h>

// =========================================================
// hw_init_gpio()
// Dipanggil saat boot, setup semua pin
// =========================================================
void hw_init_gpio(void) {

    // EN_GATE - enable gate driver IR2110
    palSetPadMode(GPIOB, 5, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPad(GPIOB, 5); // HIGH = enable

    // LED
    palSetPadMode(LED_RED_GPIO, LED_RED_PIN, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(LED_GREEN_GPIO, LED_GREEN_PIN, PAL_MODE_OUTPUT_PUSHPULL);
    LED_RED_OFF();
    LED_GREEN_OFF();

    // HALL SENSOR INPUT
    palSetPadMode(HW_HALL_ENC_GPIO1, HW_HALL_ENC_PIN1, PAL_MODE_INPUT_PULLUP);
    palSetPadMode(HW_HALL_ENC_GPIO2, HW_HALL_ENC_PIN2, PAL_MODE_INPUT_PULLUP);
    palSetPadMode(HW_HALL_ENC_GPIO3, HW_HALL_ENC_PIN3, PAL_MODE_INPUT_PULLUP);

    // UART TX/RX pins
    palSetPadMode(HW_UART_TX_PORT, HW_UART_TX_PIN,
                  PAL_MODE_ALTERNATE(GPIO_AF_USART3) |
                  PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_PULLUP);
    palSetPadMode(HW_UART_RX_PORT, HW_UART_RX_PIN,
                  PAL_MODE_ALTERNATE(GPIO_AF_USART3) |
                  PAL_STM32_OSPEED_HIGHEST | PAL_STM32_PUDR_PULLUP);
}

// =========================================================
// hw_setup_adc_channels()
// Mapping sinyal sensor ke ADC channel STM32F405
//
// STM32F405 punya 3 ADC (ADC1, ADC2, ADC3)
// VESC sampling secara sinkron semua channel sekaligus
// urutan sampling harus sesuai urutan di ADC_Value[]
//
// VERIFIKASI pin ADC dari skematik MCU-mu sebelum compile!
// =========================================================
void hw_setup_adc_channels(void) {

    // --- ADC1 regular channels ---
    // Slot 1: Current U (I_U)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0,  1, ADC_SampleTime_15Cycles);
    // Slot 2: Current V (I_V)  
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1,  2, ADC_SampleTime_15Cycles);
    // Slot 3: Voltage input (VOLT_INPUT 72V)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5,  3, ADC_SampleTime_15Cycles);
    // Slot 4: NTC temp 1 (NTC_1)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 4, ADC_SampleTime_15Cycles);
    // Slot 5: Internal Vrefint
    ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 5, ADC_SampleTime_15Cycles);

    // --- ADC2 regular channels ---
    // Slot 1: Current V (I_V) - dual sampling
    ADC_RegularChannelConfig(ADC2, ADC_Channel_1,  1, ADC_SampleTime_15Cycles);
    // Slot 2: Current W (I_W)
    ADC_RegularChannelConfig(ADC2, ADC_Channel_2,  2, ADC_SampleTime_15Cycles);
    // Slot 3: NTC temp 2 (NTC_2)
    ADC_RegularChannelConfig(ADC2, ADC_Channel_15, 3, ADC_SampleTime_15Cycles);
    // Slot 4: SENS1 (back-EMF phase U, jika dipakai)
    ADC_RegularChannelConfig(ADC2, ADC_Channel_8,  4, ADC_SampleTime_15Cycles);
    // Slot 5: SENS2 (back-EMF phase V, jika dipakai)
    ADC_RegularChannelConfig(ADC2, ADC_Channel_9,  5, ADC_SampleTime_15Cycles);

    // --- ADC3 regular channels ---
    // Slot 1: Current W (I_W) - triple sampling  
    ADC_RegularChannelConfig(ADC3, ADC_Channel_2,  1, ADC_SampleTime_15Cycles);
    // Slot 2: SENS3 (back-EMF phase W, jika dipakai)
    ADC_RegularChannelConfig(ADC3, ADC_Channel_3,  2, ADC_SampleTime_15Cycles);
    // Slot 3: Spare
    ADC_RegularChannelConfig(ADC3, ADC_Channel_10, 3, ADC_SampleTime_15Cycles);
    // Slot 4: Spare
    ADC_RegularChannelConfig(ADC3, ADC_Channel_11, 4, ADC_SampleTime_15Cycles);
    // Slot 5: Spare
    ADC_RegularChannelConfig(ADC3, ADC_Channel_12, 5, ADC_SampleTime_15Cycles);
}

// =========================================================
// hw_start_i2c() - jika pakai I2C (sensor NTC via I2C)
// =========================================================
void hw_start_i2c(void) {
    // Kamu punya I2C di skematik (SCL/SDA ke F405)
    // Implement jika ada sensor I2C yang dipakai
    // Kalau NTC pakai ADC langsung, fungsi ini kosong saja
}

void hw_stop_i2c(void) {
    // kosong
}

// =========================================================
// hw_get_temp() - baca temperatur MOSFET
// =========================================================
float hw_get_temp(void) {
    // Return dummy value dulu
    // Nanti bisa diimplementasikan setelah ADC jalan
    return 25.0;
}


void hw_try_restore_i2c(void) {
    // kosong - tidak pakai nunchuk
}

// =========================================================
// CATATAN TODO sebelum compile:
//
// 1. VERIFIKASI semua ADC channel number dari datasheet
//    STM32F405 - cek pin PA0-PA7 = CH0-CH7, PC0-PC5 = CH10-CH15
//
// 2. UKUR gain aktual op-amp MCP6002-mu dengan multimeter
//    lalu update CURRENT_AMP_GAIN di hw_kai.h
//
// 3. KONFIRMASI nilai shunt: skematik tulis 0.018R
//    artinya 18 miliOhm, update CURRENT_SHUNT_RES = 0.018
//    bukan 0.00018 seperti di komentar
//
// 4. IMPLEMENTASIKAN fault interrupt dari Trip_Signal
//    dan SD1/SD2/SD3 di fungsi ini atau di irq_handlers.c
//
// 5. TEST dengan arus rendah dulu (5-10A) sebelum full power
// =========================================================
