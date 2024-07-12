/*
 * Definitions for the MARES (LEON3) GPIO PINS
*/

#ifndef mares_gpio_defs_h_
#define mares_gpio_defs_h_

#define LX7730_ACK_IN_N_PIN                0
#define LX7730_ASPI_ENABLE_N_PIN           1
#define LX7730_ADC_CLK_OUT_PIN             2   /* Write the entire byte value to the register */
#define LX7730_RESET_OUT_N_PIN             3   /* Write the entire byte value to the register */
#define RSTO_PIN                           4
#define GPIO_B0_DIR_PIN                    5
#define GPIO_B0_OE_N_PIN                   6
#define GPIO_B1_DIR_PIN                    7
#define GPIO_B1_OE_N_PIN                   8
#define TLM_V_3P3SW_PIN                    9
#define PPS_SPARE_T_PIN                   10
#define CDH_RS422_TX5_PIN                 11
#define GPIO5_B1_PIN                      12
#define GPIO6_B1_PIN                      13
#define GPIO7_B1_PIN                      14
#define GPIO8_B1_PIN                      15
#define GPIO9_B1_PIN                      16
#define GPIO10_B1_PIN                     17
#define GPIO8_B1_DIR_PIN                  18
#define GPIO9_B1_DIR_PIN                  19
#define GPIO10_B1_DIR_PIN                 20
#define GPIO11_PIN                        21
#define GPIO12_PIN                        22
#define GPIO13_PIN                        23
#define GPIO14_PIN                        24
#define GPIO15_PIN                        25
#define SMAP_BUS0_CSI_ADV_B_OUT_N_PIN     26
#define SMAP_BUS0_CSI_PROGRAM_B_OUT_PIN   27
#define SMAP_RDWR_B_OUT_PIN               28
#define HEATER_SERVICE_1_PIN              29
#define HEATER_SERVICE_2_PIN              30
#define SPARE_1_PIN                       31
#define SPARE_2_PIN                       32

#define FSW_REG_OFFSET                    0x80500100

#define FPGA_READ_REG(addr) *((volatile uint32_t *)(addr))

#define FPGA_WRITE_REG(addr,data) *((volatile uint32_t *)(addr)) = (data)

#define FPGA_RD_REG_BITS(reg,mask,shift) \
(((FPGA_READ_REG(reg)) & mask) >> shift)

#define FPGA_WR_REG_BITS(reg,mask,shift,value)  \
FPGA_WRITE_REG(reg, (uint32_t)((FPGA_READ_REG(reg) & ~mask) | (((value) & (mask >> shift)) << shift)) )

#endif
