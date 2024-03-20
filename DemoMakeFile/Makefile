.PHONY: all build cmake format tidy check doc tests cmake-tests help flash-st flash-jlink clean clean-tests
############################### Native Makefile ###############################

PROJECT_NAME ?= firmware
BUILD_DIR ?= build
FIRMWARE := $(BUILD_DIR)/$(PROJECT_NAME).bin
BUILD_TYPE ?= Debug
PLATFORM = $(if $(OS),$(OS),$(shell uname -s))
# Device specific!
DEVICE ?= STM32F446xx
DEBUG ?= 0
ifneq ($(DEBUG),0)
C_FLAGS         += -DBLE2_DEBUG
C_FLAGS         += -DBLUENRG2_DEBUG
endif

INCLUDE_DIRS := \
    BlueNRG-2/Target \
    Core/Inc \
    Drivers/CMSIS/Device/ST/STM32F4xx/Include \
    Drivers/CMSIS/Include \
    Drivers/STM32F4xx_HAL_Driver/Inc \
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy \
    Middlewares/ST/BlueNRG-2/hci/hci_tl_patterns/Basic \
    Middlewares/ST/BlueNRG-2/includes \
    Middlewares/ST/BlueNRG-2/utils \
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 \
    Middlewares/Third_Party/FreeRTOS/Source/include \
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4

ifeq ($(PLATFORM),Windows_NT)
    BUILD_SYSTEM ?= MSYS Makefiles
else
    ifeq ($(PLATFORM),Linux)
        BUILD_SYSTEM ?= Unix Makefiles
    else
        @echo "Unsuported platform"
        exit 1
    endif
endif
CMAKE_FLAGS := -DCMAKE_C_FLAGS="-fdata-sections -ffunction-sections -specs=nano.specs -specs=nosys.specs ${C_FLAGS} -Wl,--gc-sections"
all: build

build: cmake
	$(MAKE) -C $(BUILD_DIR) --no-print-directory

cmake: $(BUILD_DIR)/Makefile

$(BUILD_DIR)/Makefile: CMakeLists.txt
	cmake $(CMAKE_FLAGS)\
		-G "$(BUILD_SYSTEM)" \
		-B$(BUILD_DIR) \
		-DPROJECT_NAME=$(PROJECT_NAME) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DDUMP_ASM=OFF

# Formats all user modified source files (add ones that are missing)
SRCS := $(shell find Core -name '*.[ch]' -or -name '*.[ch]pp')
format: $(addsuffix .format,$(SRCS))
%.format: %
	clang-format -i -style=file $<

tidy: $(addsuffix .tidy,$(SRCS))
	
%.tidy: %
	{ clang-tidy -p build -quiet $(foreach dir,$(INCLUDE_DIRS),-extra-arg=-I$(dir)) $<; } > Results/$(shell basename $<).txt
	@echo Writing log file to: Results/$(shell basename $<).txt

check: 
	cppcheck --enable=all Core 2> Results/cppcheck_analysis.txt

doc: 
	cd Docs; doxygen

tests: cmake-tests
	$(MAKE) -C Tests/googletest --no-print-directory
	cd Tests/googletest && ctest

cmake-tests: Tests/googletest/Makefile

Tests/googletest/Makefile: CMakeLists.txt
	cmake -S Tests -B Tests/googletest

help:
	@echo BT KBd documentation:
	@echo $(DEVICE)
	@echo $(DEBUG)

flash-st: build
	st-flash --reset write $(FIRMWARE) 0x08000000

$(BUILD_DIR)/jlink-script:
	touch $@
	@echo device $(DEVICE) > $@
	@echo si 1 >> $@
	@echo speed 4000 >> $@
	@echo loadfile $(FIRMWARE),0x08000000 >> $@
	@echo -e "r\ng\nqc" >> $@

flash-jlink: build | $(BUILD_DIR)/jlink-script
	JLinkExe -commanderScript $(BUILD_DIR)/jlink-script

clean:
	rm -rf $(BUILD_DIR)

clean-tests:
	rm -rf Tests/googletest

