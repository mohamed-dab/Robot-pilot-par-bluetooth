# Robot mobile IoT — STM32F407 (registres, sans HAL)
#
#   make                 compile build/robot.elf, .hex et .bin
#   make flash           programme la carte avec st-flash (stlink-tools)
#   make clean
#
# Dépendances : arm-none-eabi-gcc et les en-têtes CMSIS de ST
# (voir README, section « Compiler »).

TARGET   = robot
MCU_DEF  = STM32F407xx
CMSIS_DEVICE ?= ../cmsis_device_f4
CMSIS_CORE   ?= ../cmsis_core/CMSIS/Core/Include

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

CPU     = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS  = $(CPU) -O2 -std=c11 -Wall -Wextra -ffunction-sections -fdata-sections \
          -D$(MCU_DEF) -IInc -I$(CMSIS_DEVICE)/Include -I$(CMSIS_CORE)
LDFLAGS = $(CPU) -TSTM32F407VG.ld -Wl,--gc-sections -Wl,-Map=build/$(TARGET).map \
          --specs=nano.specs --specs=nosys.specs

SRC     = $(wildcard Src/*.c)
STARTUP = $(CMSIS_DEVICE)/Source/Templates/gcc/startup_stm32f407xx.s
OBJ     = $(patsubst Src/%.c,build/%.o,$(SRC)) build/startup.o

all: Inc/config.h build/$(TARGET).elf build/$(TARGET).hex build/$(TARGET).bin
	@$(SIZE) build/$(TARGET).elf

Inc/config.h:
	@echo "Inc/config.h absent : copie de Inc/config.example.h (pensez à renseigner vos identifiants)"
	@cp Inc/config.example.h Inc/config.h

build/%.o: Src/%.c Inc/config.h | build
	$(CC) $(CFLAGS) -c $< -o $@

build/startup.o: $(STARTUP) | build
	$(CC) $(CPU) -c $< -o $@

build/$(TARGET).elf: $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $@

build/%.hex: build/%.elf
	$(OBJCOPY) -O ihex $< $@

build/%.bin: build/%.elf
	$(OBJCOPY) -O binary $< $@

build:
	@mkdir -p build

flash: build/$(TARGET).bin
	st-flash write $< 0x08000000

clean:
	rm -rf build

.PHONY: all flash clean
