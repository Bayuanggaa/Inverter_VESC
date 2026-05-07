/*
 * hw_kai.h
 *
 * Hardware configuration untuk KAI INVERTER FUEL PUMP
 * Project ID : 2509-KAI-PIFP-A
 * MCU        : STM32F405RGT6
 * Gate Driver: IR2110 (bootstrap, tanpa SPI - TIDAK pakai DRV8301)
 * Tegangan   : 72V nominal
 * Shunt      : 0.018 Ohm x3 (low-side, per phase)
 * Dibuat dari template hw_60_core.h, disesuaikan untuk KAI
 *
 * CATATAN PENTING:
 * - TIDAK define HW_HAS_DRV8301 -> semua kode DRV di-skip otomatis
 * - Fault detection via GPIO (Trip_Signal, SD1, SD2, SD3)
 * - Current sensing gain perlu dikalibrasi (lihat bagian CURRENT SENSING)
 */

#ifndef HW_KAI_H_
#define HW_KAI_H_

// =========================================================
// NAMA HARDWARE (wajib ada, tampil di VESC Tool)
// =========================================================
#define HW_NAME                 "KAI_INVERTER"

// =========================================================
// TIDAK ADA DRV - jangan uncomment baris di bawah ini
// =========================================================
// #define HW_HAS_DRV8301    // <-- JANGAN! kamu pakai IR2110
// #define HW_HAS_DRV8305
// #define HW_HAS_DRV8320S

// =========================================================
// TEGANGAN INPUT
// Voltage divider dari skematik sensor (sheet 6):
//   R84 = 160k, R85 = 160k (seri) = 320k total ke +72V
//   R86 = 10k ke GND
//   Rasio = (320k + 10k) / 10k = 33
//   Rumus: V_input = ADC_volt * (VIN_R1 + VIN_R2) / VIN_R2
// =========================================================
#define VIN_R1                  320000.0   // R84+R85 seri = 160k+160k
#define VIN_R2                  10000.0    // R86
#define V_REG                   3.3        // referensi ADC 3.3V

#define GET_INPUT_VOLTAGE()     ((V_REG / 4095.0) * (float)ADC_Value[ADC_IND_VIN_SENS] \
                                * ((VIN_R1 + VIN_R2) / VIN_R2))

// Batas tegangan (72V nominal, toleransi +-10%)
#define HW_LIM_VIN_MIN          10.0
#define HW_LIM_VIN_MAX          90.0       // batas atas aman
#define MCCONF_L_MIN_VOLTAGE    10.0
#define MCCONF_L_MAX_VOLTAGE    85.0

// =========================================================
// CURRENT SENSING
// Shunt: 0.018 Ohm (R15, R16, R17 di sheet MOSFET)
// Op-amp: MCP6002 dengan gain dari resistor divider
//
// PERINGATAN: Cek ulang gain-mu!
// Dengan shunt 0.018R dan gain ~3.5x:
//   100A -> 1.8V x 3.5 = 6.3V -> MELEBIHI 3.3V ADC!
// Gunakan gain lebih rendah, sekitar 1.5-1.8x:
//   100A -> 1.8V x 1.8 = 3.24V -> aman
//
// Set nilai ini sesuai gain aktual op-amp kamu setelah diukur
// =========================================================
#define CURRENT_AMP_GAIN        20.0       // SESUAIKAN! ukur gain aktual dulu
#define CURRENT_SHUNT_RES       0.00018    // 0.018 mOhm -> 0.00018 Ohm? 
                                           // CEK: 0.018R atau 0.018mR?
                                           // Dari skematik tertulis 0.018R

// Batas arus (sesuaikan dengan rating IRFP260N dan aplikasi)
#define HW_LIM_CURRENT          150.0
#define HW_LIM_CURRENT_IN       150.0
#define MCCONF_L_MAX_ABS_CURRENT 200.0

// =========================================================
// PIN PWM OUTPUT ke IR2110
// Dari sheet MCU (STM32F405), signal ke gate driver:
//   IN_UH -> PA8  (TIM1_CH1) - Phase U High
//   IN_VH -> PA9  (TIM1_CH2) - Phase V High  
//   IN_WH -> PA10 (TIM1_CH3) - Phase W High
//   IN_UL -> PB13 (TIM1_CH1N) - Phase U Low
//   IN_VL -> PB14 (TIM1_CH2N) - Phase V Low
//   IN_WL -> PB15 (TIM1_CH3N) - Phase W Low
//
// VERIFIKASI: Cek SolderJumper UL1 dan WL1 di skematik
// apakah IN_UL dan IN_WL terhubung langsung atau via jumper
// =========================================================
#define HW_HAS_3_SHUNTS         // 3 shunt low-side (U, V, W)

// TIM1 untuk PWM center-aligned (wajib untuk FOC)
// Pin ini sudah fixed di STM32F405 hardware, tidak perlu define ulang
// VESC firmware otomatis pakai TIM1 CH1/2/3 dan CH1N/2N/3N

// EN_GATE: dari skematik F405, ada sinyal EN_GATE
// Di IR2110 tidak ada EN pin seperti DRV, tapi kamu bisa pakai
// sebagai enable/disable logic sebelum gate driver
#define HW_HAS_GATE_DRIVER_ENABLE // uncomment kalau EN_GATE disambung ke GPIO

// =========================================================
// PIN ADC - CURRENT SENSING
// Dari sheet MCU (STM32F405):
//   I_U -> ADC0 (konfirmasi pin aktual dari skematik)
//   I_V -> ADC1
//   I_W -> ADC2
// =========================================================
#define ADC_IND_CURR1           0          // I_U -> ADC channel 0
#define ADC_IND_CURR2           1          // I_V -> ADC channel 1
#define ADC_IND_CURR3           2          // I_W -> ADC channel 2

// =========================================================
// PIN ADC - VOLTAGE & TEMPERATURE
// Dari sheet MCU:
//   VOLT_INPUT -> ADC3 (voltage sensing 72V)
//   SENS1/SENS2/SENS3 -> back-EMF sensor (jika dipakai)
//   NTC_1 -> ADC untuk temp sensor 1
//   NTC_2 -> ADC untuk temp sensor 2
// =========================================================
#define ADC_IND_VIN_SENS        3          // VOLT_INPUT
#define ADC_IND_TEMP_MOS        4          // NTC_1 (MOSFET temp)
#define ADC_IND_TEMP_MOTOR      5          // NTC_2 (motor temp)

// =========================================================
// HALL SENSOR
// Dari sheet MCU dan COM (STM32F405):
//   HAL_U -> PC6 (TIM3_CH1)
//   HAL_V -> PC7 (TIM3_CH2)
//   HAL_W -> PC8 (TIM3_CH3)
// Verifikasi pin dari sheet MCU skematik-mu
// =========================================================
#define HW_HAS_PHASE_FILTERS    // kamu punya filter RC di Hall sensor

// Hall GPIO (konfirmasi dari skematik MCU sheet)
#define HW_HALL_ENC_GPIO1       GPIOC
#define HW_HALL_ENC_PIN1        6          // HAL_U -> PC6
#define HW_HALL_ENC_GPIO2       GPIOC
#define HW_HALL_ENC_PIN2        7          // HAL_V -> PC7
#define HW_HALL_ENC_GPIO3       GPIOC
#define HW_HALL_ENC_PIN3        8          // HAL_W -> PC8

// =========================================================
// CAN BUS
// Dari sheet COM:
//   CAN_TX -> PA12 (CAN1_TX) atau PB9
//   CAN_RX -> PA11 (CAN1_RX) atau PB8
// SN65HVD232 sudah terhubung ke CAN1 STM32F405
// =========================================================
// VESC firmware otomatis handle CAN1 jika pin standard dipakai

// =========================================================
// UART / SERIAL
// Dari sheet COM:
//   TX -> USART1_TX
//   RX -> USART1_RX
// =========================================================
// Otomatis dihandle VESC firmware

// =========================================================
// LED
// Dari sheet COM:
//   LED_RED   -> GPIO output
//   LED_GREEN -> GPIO output
// Konfirmasi pin dari skematik MCU
// =========================================================
// Define ini kalau kamu mau LED aktif di firmware
// #define HW_LED_RED_GPIO     GPIOA
// #define HW_LED_RED_PIN      x
// #define HW_LED_GREEN_GPIO   GPIOA  
// #define HW_LED_GREEN_PIN    x

// =========================================================
// FAULT / TRIP SIGNAL
// Dari sheet MCU:
//   Trip_Signal -> input GPIO dari sensor board
//   Fault       -> output/input fault
//   SD1, SD2, SD3 -> shutdown signals dari current sensing
//
// Karena tidak ada DRV, fault harus dideteksi manual
// via interrupt GPIO dari sinyal-sinyal ini
// =========================================================
// Ini perlu diimplementasikan di hw_kai.c

// =========================================================
// NTC THERMISTOR
// Dari sheet sensor:
//   NTC1, NTC2 pakai JST_XH connector
//   R89 = 10k pullup ke 3V3
//   R87 = 10 (seri)
//   C45 = 100nF filter
// Formula NTC standard (Beta = 3380 typical, konfirmasi datasheet NTC-mu)
// =========================================================
#define NTC_RES(adc_val)        (10000.0 / ((4095.0 / (float)adc_val) - 1.0))
#define NTC_TEMP(adc_ind)       (1.0 / ((logf(NTC_RES(ADC_Value[adc_ind]) \
                                / 10000.0) / 3380.0) \
                                + (1.0 / 298.15)) - 273.15)

// Batas temperatur
#define HW_LIM_TEMP_FET_START   80.0       // mulai kurangi current
#define HW_LIM_TEMP_FET_END     100.0      // shutdown
#define MCCONF_L_TEMP_FET_START 80.0
#define MCCONF_L_TEMP_FET_END   100.0

// =========================================================
// PWM SWITCHING FREQUENCY
// IR2110 bisa handle sampai ~500kHz, tapi untuk motor 3-phase
// FOC optimal di 10-30kHz. Mulai dengan 20kHz.
// =========================================================
#define MCCONF_FOC_F_SW         20000.0    // 20 kHz switching freq

// =========================================================
// DEAD TIME
// IR2110 punya built-in dead time yang sangat kecil (<100ns)
// VESC firmware juga inject dead time software
// Mulai dengan nilai aman 2.5us, sesuaikan setelah test
// =========================================================
#define HW_DEAD_TIME_NSEC       2500.0     // 2.5 microsecond

// =========================================================
// DEFAULT MOTOR CONFIG (bisa diubah via VESC Tool)
// =========================================================
#define MCCONF_DEFAULT_MOTOR_TYPE   MOTOR_TYPE_FOC
#define MCCONF_L_CURRENT_MAX        50.0   // mulai konservatif dulu
#define MCCONF_L_CURRENT_MIN        -50.0
#define MCCONF_L_IN_CURRENT_MAX     50.0
#define MCCONF_L_IN_CURRENT_MIN     -50.0

#endif /* HW_KAI_H_ */
