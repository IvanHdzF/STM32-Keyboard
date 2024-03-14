.PHONY: all build build-container cmake format format-linux flash-stlink flash-jlink format-container shell image build-container clean clean-image clean-all
############################### Native Makefile ###############################

PROJECT_NAME ?= firmware
BUILD_DIR ?= build
FIRMWARE := $(BUILD_DIR)/$(PROJECT_NAME).bin
BUILD_TYPE ?= Debug
PLATFORM = $(if $(OS),$(OS),$(shell uname -s))

ifeq ($(PLATFORM),Windows_NT)
    BUILD_SYSTEM ?= MinGW Makefiles
else
    ifeq ($(PLATFORM),Linux)
        BUILD_SYSTEM ?= Unix Makefiles
    else
        @echo "Unsuported platform"
        exit 1
    endif
endif

all: build

build: cmake
	$(MAKE) -C $(BUILD_DIR) --no-print-directory

cmake: $(BUILD_DIR)/Makefile

$(BUILD_DIR)/Makefile: CMakeLists.txt
	cmake \
		-G "$(BUILD_SYSTEM)" \
		-B$(BUILD_DIR) \
		-DPROJECT_NAME=$(PROJECT_NAME) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DDUMP_ASM=OFF

# Formats all user modified source files (add ones that are missing)
SRCS := $(shell find Core -name '*.[ch]' -or -name '*.[ch]pp')
format: $(addsuffix .format,$(SRCS))
%.format: %
	clang-format -i $<


# Device specific!
DEVICE ?= STM32F446xx

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

