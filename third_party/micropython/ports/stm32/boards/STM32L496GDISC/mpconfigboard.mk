MCU_SERIES = l4
CMSIS_MCU = STM32L496xx
AF_FILE = boards/stm32l496_af.csv
LD_FILES = boards/stm32l496xg.ld boards/common_ifs.ld
TEXT0_ADDR = 0x08000000
TEXT1_ADDR = 0x08004000
OPENOCD_CONFIG = boards/openocd_stm32l4.cfg
