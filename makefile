##########################################
## GLOBAL

MAKEFLAGS += --silent
TARGET 	:= crot
CC		:= /usr/bin/gcc

# Flags
INCS	:= -I./src/ -I ~/Documents/Dependencies/clay/
INCS	+= -L/usr/lib/
LIBS	:= -lpthread -lm -ldl -lraylib
MKF_DIR	:= $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR	:= $(MKF_DIR:makefile=)

# Build
BUILD := build
OBJS  :=
OBJS  += $(patsubst %.c,$(BUILD)/%.c.o, $(shell find ./src -name "*.c"))
OBJS  += $(patsubst %.cpp,$(BUILD)/%.cpp.o, $(shell find ./src -name "*.cpp"))
HEAD  := $(patsubst %.h,$(BUILD)/inc/%.h, $(shell find ./src -name "*.h" ! -name "*.internal.h"))

##########################################
## Processes

CPU_COUNT=$(shell grep -c "^processor" /proc/cpuinfo)

##########################################
## Rules

all-dbg: $(BUILD)/$(TARGET)-dbg
all-rel: $(BUILD)/$(TARGET)-rel

rel:
	$(MAKE) -j$(CPU_COUNT) all-rel
dbg:
	$(MAKE) -j$(CPU_COUNT) all-dbg

# debug excecutiable
$(BUILD)/$(TARGET)-dbg: $(OBJS)
	$(CC) -g $(CFLAGS) $(OBJS) -o $(BUILD)/$(TARGET) $(INCS) $(LIBS) $(LDFLAGS)

# release excecutiable
$(BUILD)/$(TARGET)-rel: $(OBJS)
	$(CC) -O3 $(CFLAGS) $(OBJS) -o $(BUILD)/$(TARGET) $(INCS) $(LIBS) $(LDFLAGS)
	@printf "[$$(date +%H:%M:%S)][\033[32mMAKE\033[0m] Sripping binary\n"
	strip $(BUILD)/$(TARGET)

# c++ source
$(BUILD)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CC) -c -fPIC $< -o $@ $(INCS) $(LIBS) $(CFLAGS)

# c source
$(BUILD)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(INCS) $(LIBS) $(CFLAGS)
	@printf "[$$(date +%H:%M:%S)][\033[35mBUILD\033[0m] $$(basename ${CC}) $<\n"

##########################################
## Commands

.PHONY: run
run:
	@printf "[$$(date +%H:%M:%S)][\033[32mMAKE\033[0m] Running './$(BUILD)/$(TARGET)'\n"
	$(BUILD)/$(TARGET)

.PHONY: leak
leak:
	valgrind --leak-check=full --track-origins=yes $(BUILD)/$(TARGET)

.PHONY: gdb
gdb:
	gdb $(BUILD)/$(TARGET)

.PHONY: clean
clean:
	mkdir -p $(BUILD)
	rm -r $(BUILD)
	@printf "[$$(date +%H:%M:%S)][\033[32mMAKE\033[0m] Cleaning $(BUILD)\n"
