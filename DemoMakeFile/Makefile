.PHONY: all build build-container cmake format format-linux flash-stlink flash-jlink format-container shell image build-container clean clean-image clean-all
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
	@echo Flags are: $(C_FLAGS)

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

