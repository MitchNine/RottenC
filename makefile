########################################## Settings
MAKEFLAGS += --silent
TARGET 	  := sample_application
BUILD     := build
CC        := /usr/bin/gcc
INCS      := -I./src/ -L/usr/lib/ -I/usr/includes/
LIBS      :=

########################################## Global
MKF_DIR	:= $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR	:= $(MKF_DIR:makefile=)
OBJS := $(patsubst %.c,$(BUILD)/%.c.o, $(shell find ./src -name "*.c"))
OBJS += $(patsubst %.cpp,$(BUILD)/%.cpp.o, $(shell find ./src -name "*.cpp"))
CPU_COUNT=$(shell grep -c "^processor" /proc/cpuinfo)

########################################## Rules
all-dbg: $(BUILD)/$(TARGET)-dbg
all-rel: $(BUILD)/$(TARGET)-rel
rel:
	$(MAKE) -j$(CPU_COUNT) all-rel
dbg:
	$(MAKE) -j$(CPU_COUNT) all-dbg
$(BUILD)/$(TARGET)-dbg: $(OBJS)
	$(CC) -g $(CFLAGS) $(OBJS) -o $(BUILD)/$(TARGET) $(INCS) $(LIBS) $(LDFLAGS)
$(BUILD)/$(TARGET)-rel: $(OBJS)
	$(CC) -O3 $(CFLAGS) $(OBJS) -o $(BUILD)/$(TARGET) $(INCS) $(LIBS) $(LDFLAGS)
	strip $(BUILD)/$(TARGET)
$(BUILD)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CC) -c -fPIC $< -o $@ $(INCS) $(LIBS) $(CFLAGS)
	@if [ -t 1 ]; then printf "[$$(date +%H:%M:%S)][\033[35mBUILD\033[0m] $$(basename ${CC}) $<\n"; \
	else printf "[$$(date +%H-%M-%S)][BUILD] $$(basename ${CC}) $<\n"; fi
$(BUILD)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) -g -c $< -o $@ $(INCS) $(LIBS) $(CFLAGS)
	@if [ -t 1 ]; then printf "[$$(date +%H:%M:%S)][\033[35mBUILD\033[0m] $$(basename ${CC}) $<\n"; \
	else printf "[$$(date +%H-%M-%S)][BUILD] $$(basename ${CC}) $<\n"; fi

########################################## Commands
.PHONY: run
run:
	@if [ -t 1 ]; then printf "[$$(date +%H:%M:%S)][\033[34mMAKE\033[0m] Running './$(BUILD)/$(TARGET)'\n"; \
	else printf "[$$(date +%H-%M-%S)][MAKE] Running './$(BUILD)/$(TARGET)'\n"; fi
	$(BUILD)/$(TARGET)
.PHONY: clean
clean:
	mkdir -p $(BUILD)
	rm -r $(BUILD)
	@if [ -t 1 ]; then printf "[$$(date +%H:%M:%S)][\033[34mMAKE\033[0m] Cleaning $(BUILD)\n"; \
	else printf "[$$(date +%H-%M-%S)][MAKE] Cleaning $(BUILD)\n"; fi
