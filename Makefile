COMPILER := gcc
FLAGS := -g -O3
BUILD_DIR := build
EXECUTABLE := netperf

FILES := main.c netutils.c traceroute.c ll.c stats.c
OBJECTS := $(addprefix $(BUILD_DIR)/,$(patsubst %.c,%.o,$(FILES)))

.PHONY: $(EXECUTABLE)
$(EXECUTABLE): $(BUILD_DIR)/$(EXECUTABLE)

.PRECIOUS: $(BUILD_DIR)/. $(BUILD_DIR)%/.

$(BUILD_DIR)/.:
	mkdir -p $@

$(BUILD_DIR)%/.:
	mkdir -p $@

.SECONDEXPANSION:

$(BUILD_DIR)/%.o: src/%.c | $$(@D)/.
	$(COMPILER) -c $(FLAGS) $< -o $@

$(BUILD_DIR)/$(EXECUTABLE): $(OBJECTS)
	$(COMPILER) $^ -o ./$(BUILD_DIR)/$(EXECUTABLE)

run:
	make
	sudo ./$(BUILD_DIR)/netperf stevens.edu

new: clean
	make

clean:
	rm -rf build
