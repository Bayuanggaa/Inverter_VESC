/*
 * hw_kai.h
 * Hardware configuration untuk KAI INVERTER FUEL PUMP
 * Project  : 2509-KAI-PIFP-A
 * MCU      : STM32F405RGT6
 * Driver   : IR2110 (bukan DRV8301)
 * Tegangan : 72V nominal
 * Shunt    : 0.018 Ohm x3 (low-side per phase)
 * Base     : VESC firmware v6.05
 */

#ifndef HW_KAI_H_
#define HW_KAI_H_

#define HW_NAME                 "KAI_INVERTER_72V"

// TIDAK PAKAI DRV - jangan uncomment
// #define HW_HAS_DRV8301
// #define HW_HAS_DRV8305

// 3 SHUNT low-side per phase
#define HW_HAS_3_SHUNTS
#define HW_HAS_PHASE_SHUNTS

// Tegangan input
// Voltage divider: R84+R85 = 320k, R86 = 10k, rasio = 33
#define VIN_R1                  320000.0
#define VIN_R2                  10000.0
#define V_REG                   3.3
#define HW_LIM_VIN_MIN          10.0
#define HW_LIM_VIN_MAX          90.0
#define MCCONF_L_MIN_VOLTAGE    10.0
#define MCCONF_L_MAX_VOLTAGE    85.0

// Current sensing
// Shunt 0.018 Ohm + MCP6002 op-amp
// SESUAIKAN CURRENT_AMP_GAIN setelah ukur gain aktual!
#define CURRENT_SHUNT_RES       0.018
#define CURRENT_AMP_GAIN        10.0
#define MCCONF_L_MAX_ABS_CURRENT 150.0
#define MCCONF_L_CURRENT_MAX    50.0
#define MCCONF_L_CURRENT_MIN    -50.0
#define MCCONF_L_IN_CURRENT_MAX 50.0
#define MCCONF_L_IN_CURRENT_MIN -10.0

// ADC channel mapping
#define ADC_IND_CURR1           0
#define ADC_IND_CURR2           1
#define ADC_IND_CURR3           2
#define ADC_IND_VIN_SENS        3
#define ADC_IND_TEMP_MOS        4
#define ADC_IND_TEMP_MOTOR      5
#define ADC_IND_EXT             6
#define ADC_IND_EXT2            ADC_IND_EXT
#define ADC_IND_EXT3            ADC_IND_EXT
#define ADC_IND_VREFINT         7
#define HW_ADC_CHANNELS         8
#define HW_ADC_INJ_CHANNELS     3
#define HW_ADC_NBR_CONV         4

#define ADC_VOLTS(ch)           ((float)ADC_Value[ch] / 4095.0 * V_REG)

#define GET_INPUT_VOLTAGE()     ((V_REG / 4095.0) * (float)ADC_Value[ADC_IND_VIN_SENS] \
                                * ((VIN_R1 + VIN_R2) / VIN_R2))

// NTC Thermistor (10k pullup, Beta=3380)
#define NTC_RES(adc_val)        ((V_REG * 10000.0) / \
                                ((V_REG / ((float)adc_val / 4095.0 * V_REG)) - V_REG))
#define NTC_TEMP(adc_ind)       (1.0 / ((logf(NTC_RES(ADC_Value[adc_ind]) / 10000.0) \
                                / 3380.0) + (1.0 / 298.15)) - 273.15)
#define NTC_RES_MOTOR(adc_val)  NTC_RES(adc_val)
#define NTC_TEMP_MOTOR(adc_val) NTC_TEMP(ADC_IND_TEMP_MOTOR)
#define HW_LIM_TEMP_FET_START   80.0
#define HW_LIM_TEMP_FET_END     100.0
#define HW_LIM_TEMP_MOTOR_START 80.0
#define HW_LIM_TEMP_MOTOR_END   100.0
#define MCCONF_L_TEMP_FET_START 80.0
#define MCCONF_L_TEMP_FET_END   100.0

// PWM switching frequency dan dead time
#define HW_DEAD_TIME_NSEC       2500.0
#define MCCONF_FOC_F_ZV    20000.0

// Hall sensor pins
// HAL_U->PC6, HAL_V->PC7, HAL_W->PC8
#define HW_HALL_ENC_GPIO1       GPIOC
#define HW_HALL_ENC_PIN1        6
#define HW_HALL_ENC_GPIO2       GPIOC
#define HW_HALL_ENC_PIN2        7
#define HW_HALL_ENC_GPIO3       GPIOC
#define HW_HALL_ENC_PIN3        8
#define READ_HALL1()            palReadPad(HW_HALL_ENC_GPIO1, HW_HALL_ENC_PIN1)
#define READ_HALL2()            palReadPad(HW_HALL_ENC_GPIO2, HW_HALL_ENC_PIN2)
#define READ_HALL3()            palReadPad(HW_HALL_ENC_GPIO3, HW_HALL_ENC_PIN3)

// Encoder / EXTI - wajib ada meski tidak pakai encoder
#define HW_ENC_TIM              TIM4
#define HW_ENC_TIM_AF           GPIO_AF_TIM4
#define HW_ENC_EXTI_LINE        EXTI_Line8
#define HW_ENC_EXTI_CH          EXTI9_5_IRQn
#define HW_ENC_EXTI_ISR_VEC     EXTI9_5_IRQHandler
#define HW_ENC_TIM_ISR_VEC      TIM4_IRQHandler
#define HW_ENC_TIM_CLK_EN()     RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE)

// ICU - wajib ada meski tidak dipakai
#define HW_ICU_TIMER            TIM9
#define HW_ICU_GPIO             GPIOA
#define HW_ICU_PIN              2
#define HW_ICU_GPIO_AF          GPIO_AF_TIM9
#define HW_ICU_CHANNEL          ICU_CHANNEL_1
#define HW_ICU_BUS              STM32_ICU_TIM9

// LED - sesuaikan pin dari skematik COM
#define LED_RED_GPIO            GPIOA
#define LED_RED_PIN             13
#define LED_GREEN_GPIO          GPIOA
#define LED_GREEN_PIN           14
#define LED_RED_ON()            palSetPad(LED_RED_GPIO, LED_RED_PIN)
#define LED_RED_OFF()           palClearPad(LED_RED_GPIO, LED_RED_PIN)
#define LED_GREEN_ON()          palSetPad(LED_GREEN_GPIO, LED_GREEN_PIN)
#define LED_GREEN_OFF()         palClearPad(LED_GREEN_GPIO, LED_GREEN_PIN)

// Gate enable dan fault
// EN_GATE -> PB5 (sesuaikan dari skematik)
#define ENABLE_GATE()           palSetPad(GPIOB, 5)
#define DISABLE_GATE()          palClearPad(GPIOB, 5)
#define IS_DRV_FAULT()          0

// SPI dummy - tidak dipakai tapi wajib ada
#define HW_SPI_DEV              SPID1
#define HW_SPI_GPIO_AF          GPIO_AF_SPI1
#define HW_SPI_PORT_NSS         GPIOA
#define HW_SPI_PIN_NSS          4
#define HW_SPI_PORT_SCK         GPIOA
#define HW_SPI_PIN_SCK          5
#define HW_SPI_PORT_MOSI        GPIOA
#define HW_SPI_PIN_MOSI         7
#define HW_SPI_PORT_MISO        GPIOA
#define HW_SPI_PIN_MISO         6

// Default motor type
#define MCCONF_DEFAULT_MOTOR_TYPE   MOTOR_TYPE_FOC

// Function declarations
void hw_init_gpio(void);
void hw_setup_adc_channels(void);
void hw_start_i2c(void);
void hw_stop_i2c(void);
float hw_get_temp(void);

// =========================================================
// UART - USART3 (TX=PC10, RX=PC11)
// Dari skematik MCU F405: TX di pin64=PC10, RX di pin66=PC11
// =========================================================
// I2C - SCL=PB10, SDA=PB11 (dari skematik MCU F405)
#define HW_I2C_DEV              I2CD2
#define HW_I2C_GPIO_AF          GPIO_AF_I2C2
#define HW_I2C_SCL_PORT         GPIOB
#define HW_I2C_SCL_PIN          10
#define HW_I2C_SDA_PORT         GPIOB
#define HW_I2C_SDA_PIN          11

#define HW_UART_DEV             SD3
#define HW_UART_BAUDRATE        115200
#define HW_UART_TX_PORT         GPIOC
#define HW_UART_TX_PIN          10
#define HW_UART_RX_PORT         GPIOC
#define HW_UART_RX_PIN          11
#define HW_UART_AF              GPIO_AF_USART3
#define HW_UART_GPIO_AF         GPIO_AF_USART3 
#define HW_CAN_GPIO_AF          GPIO_AF_CAN1

// CAN - dari skematik COM: CAN_TX dan CAN_RX ke F405
// CAN_TX -> PB9 (CAN1_TX), CAN_RX -> PB8 (CAN1_RX)
#define HW_CAN_DEV              CAND1
#define HW_CAN_AF               GPIO_AF_CAN1
#define HW_CANTX_PORT           GPIOB
#define HW_CANTX_PIN            9
#define HW_CANRX_PORT           GPIOB
#define HW_CANRX_PIN            8
#define HW_ENC_TIM_ISR_CH       TIM4_IRQn
#define HW_ENC_EXTI_PORTSRC     EXTI_PortSourceGPIOC
#define HW_ENC_EXTI_PINSRC      EXTI_PinSource8


#endif /* HW_KAI_H_ */
