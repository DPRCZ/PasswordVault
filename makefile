 # ===================================================================================
# Project:  CH55x Makefile Template 
# Author:   
# Modification: 
# Year:     
# URL:      
# ===================================================================================         
# Type "make help" in the command line.
# ===================================================================================

# Files and directories
SRC_DIR    = src
BUILD_DIR  = build
OUTPUT_DIR = .

MAINFILE   = main.c
TARGET     = passvalut
INCLUDE    = include 
TOOLS      = tools

# Microcontroller configuration
FREQ_SYS   = 24000000
XRAM_LOC   = 80
XRAM_SIZE  = 1024
CODE_SIZE  = 0x3800

# Toolchain
CC         = sdcc
OBJCOPY    = objcopy
PACK_HEX   = packihx

# Compilation flags
CFLAGS  = -mmcs51 --model-large -DF_CPU=$(FREQ_SYS) -I$(INCLUDE) -I. -I$(SRC_DIR)
CFLAGS += --xram-size $(XRAM_SIZE) --xram-loc $(XRAM_LOC) --code-size $(CODE_SIZE)

# Source files and objects
CFILES  = $(MAINFILE) $(wildcard $(SRC_DIR)/*.c) 
# Unique list of source files to avoid compiling mainfile twice if wildcard catches it
UNIQUE_CFILES = $(sort $(CFILES))
RFILES  = $(patsubst %.c,$(BUILD_DIR)/%.rel,$(notdir $(UNIQUE_CFILES)))

# Files to clean up
CLEAN_FILES = *.ihx *.lk *.map *.mem *.lst *.rel *.rst *.sym *.asm *.adb

vpath %.c $(SRC_DIR) $(INCLUDE)

# Show help
help:
	@echo "Available commands:"
	@echo "make all     -> Compile and build everything"
	@echo "make hex     -> Generate $(OUTPUT_DIR)/$(TARGET).hex"
	@echo "make bin     -> Generate $(OUTPUT_DIR)/$(TARGET).bin"
	@echo "make clean   -> Clean generated files"

# Ensure directories exist
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Pattern rule for compiling C files from src/ or include/ into build/
$(BUILD_DIR)/%.rel: %.c | $(BUILD_DIR)
	@echo "Compiling $< ..."
	@$(CC) -c $(CFLAGS) -o $@ $<

# Linking into build directory
$(BUILD_DIR)/$(TARGET).ihx: $(RFILES)
	@echo "Linking IHX..."
	@$(CC) $(RFILES) $(CFLAGS) -o $@

# Generate HEX in output directory
$(OUTPUT_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).ihx
	@echo "Generating HEX..."
	@$(PACK_HEX) $< > $@

# Generate BIN in output directory
$(OUTPUT_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).ihx
	@echo "Generating BIN..."
	@$(OBJCOPY) -I ihex -O binary $< $@

# Standard targets
all: $(OUTPUT_DIR)/$(TARGET).bin $(OUTPUT_DIR)/$(TARGET).hex size

hex: $(OUTPUT_DIR)/$(TARGET).hex size removetemp

bin: $(OUTPUT_DIR)/$(TARGET).bin size removetemp

bin-hex: $(OUTPUT_DIR)/$(TARGET).bin $(OUTPUT_DIR)/$(TARGET).hex size removetemp

# Show memory usage
size:
	@echo "------------------"
	@echo "FLASH: $(shell awk '$$1 == "ROM/EPROM/FLASH" {print $$4}' $(BUILD_DIR)/$(TARGET).mem) bytes"
	@echo "IRAM:  $(shell awk '$$1 == "Stack" {print 248-$$10}' $(BUILD_DIR)/$(TARGET).mem) bytes"
	@echo "XRAM:  $(shell awk '$$1 == "EXTERNAL" {print $(XRAM_LOC)+$$5}' $(BUILD_DIR)/$(TARGET).mem) bytes"
	@echo "------------------"

# Cleanup
removetemp:
	@echo "Removing temporary files..."
	@rm -f $(addprefix $(BUILD_DIR)/,$(CLEAN_FILES))

clean:
	@echo "Cleaning everything..."
	@rm -rf $(BUILD_DIR)

