########################################## Settings
MAKEFLAGS += --silent
TARGET 	  := sample_application
BUILD     := build
CC        := /usr/bin/gcc
INCS      := -I./src/ -L/usr/lib/ -I/usr/includes/
LIBS      :=

LDFLAGS=$(INCS) $(LIBS)

CCFLAGS    += -Wall
CXXFLAGS   += -Wall

########################################## Global
MKF_DIR	:= $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR	:= $(MKF_DIR:makefile=)
OBJS := $(patsubst %.c,$(BUILD)/%.c.o, $(shell find ./src -name "*.c"))
OBJS += $(patsubst %.cpp,$(BUILD)/%.cpp.o, $(shell find ./src -name "*.cpp"))
CPU_COUNT=$(shell grep -c "^processor" /proc/cpuinfo)

########################################## Rules

all: $(BUILD)/$(TARGET)

dbg: CCFLAGS    += -g -pg -DDEBUG -fsanitize=address
dbg: CXXFLAGS   += -g -pg -DDEBUG -fsanitize=address
dbg: $(BUILD)/$(TARGET)
rel: CCFLAGS    += -O3
rel: CXXFLAGS   += -O3
rel: $(BUILD)/$(TARGET)
	@if [ -t 1 ]; then printf "[$$(date +%H:%M:%S)][\033[34mMAKE\033[0m] Stripping './$(BUILD)/$(TARGET)'\n"; \
	else printf "[$$(date +%H-%M-%S)][MAKE] Running './$(BUILD)/$(TARGET)'\n"; fi
	strip $(BUILD)/$(TARGET)

$(BUILD)/$(TARGET): $(OBJS)
	$(CXX) $(CCFLAGS) $(OBJS) -o $(BUILD)/$(TARGET) $(LDFLAGS)

$(BUILD)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CC) -fPIC $< -o $@ $(LDFLAGS) $(CCFLAGS)
	@if [ -t 1 ]; then printf "[$$(date +%H:%M:%S)][\033[35mBUILD\033[0m] $$(basename ${CC}) $<\n"; \
	else printf "[$$(date +%H-%M-%S)][BUILD] $$(basename ${CC}) $<\n"; fi
$(BUILD)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(LDFLAGS) $(CCFLAGS)
	@if [ -t 1 ]; then printf "[$$(date +%H:%M:%S)][\033[35mBUILD\033[0m] $$(basename ${CC}) $<\n"; \
	else printf "[$$(date +%H-%M-%S)][BUILD] $$(basename ${CC}) $<\n"; fi

########################################## Commands
.PHONY: run
run:
	@if [ -t 1 ]; then printf "[$$(date +%H:%M:%S)][\033[34mMAKE\033[0m] Running './$(BUILD)/$(TARGET)'\n"; \
	else printf "[$$(date +%H-%M-%S)][MAKE] Running './$(BUILD)/$(TARGET)'\n"; fi
	$(BUILD)/$(TARGET) $(ARGS)
.PHONY: strip
strip:
	@if [ -t 1 ]; then printf "[$$(date +%H:%M:%S)][\033[34mMAKE\033[0m] Stripping './$(BUILD)/$(TARGET)'\n"; \
	else printf "[$$(date +%H-%M-%S)][MAKE] Running './$(BUILD)/$(TARGET)'\n"; fi
	strip $(BUILD)/$(TARGET)
.PHONY: clean
clean:
	mkdir -p $(BUILD)
	rm -r $(BUILD)
	@if [ -t 1 ]; then printf "[$$(date +%H:%M:%S)][\033[34mMAKE\033[0m] Cleaning $(BUILD)\n"; \
	else printf "[$$(date +%H-%M-%S)][MAKE] Cleaning $(BUILD)\n"; fi
