# Directory layout
SRC_SERVERS   := $(wildcard src/servers/*.c)
SRC_CONCUR    := $(wildcard src/concurrency/*.c)
SRC_TOOLS     := $(wildcard src/tools/*.c)
SRC_DATASTR   := $(wildcard src/datastruct/*.c)

COMMON_DIR    := common
THIRD_PARTY   := third_party
BUILD_DIR     := build

CC      := gcc
CFLAGS  := -g -Wall -I$(COMMON_DIR) -I$(THIRD_PARTY)
LDFLAGS := -L./lib

# libuv is optional: uv based servers are built only when libuv is installed
UV_H := $(firstword $(wildcard /usr/include/uv.h /usr/local/include/uv.h))

# targets
SERVER_BINS     := $(patsubst %.c,$(BUILD_DIR)/%,$(SRC_SERVERS))
ifneq ($(UV_H),)
SERVER_BINS_ALL := $(SERVER_BINS)
else
SERVER_BINS_ALL := $(filter-out $(BUILD_DIR)/src/servers/uv_server $(BUILD_DIR)/src/servers/uv_prime_server,$(SERVER_BINS))
endif
CONCURRENCY_BIN := $(patsubst %.c,$(BUILD_DIR)/%,$(SRC_CONCUR))
TOOLS_BINS      := $(patsubst %.c,$(BUILD_DIR)/%,$(SRC_TOOLS))
DATASTRUCT_BINS := $(patsubst %.c,$(BUILD_DIR)/%,$(SRC_DATASTR))

COMMON_OBJ := $(BUILD_DIR)/utils.o

.PHONY: all servers concurrency tools datastruct clean

all: servers concurrency tools datastruct

servers:     $(SERVER_BINS_ALL)
concurrency: $(CONCURRENCY_BIN)
tools:       $(TOOLS_BINS)
datastruct:  $(DATASTRUCT_BINS)

# ---- link rule for all binaries (common/utils.o is harmless when unused) ----
$(BUILD_DIR)/%: $(BUILD_DIR)/%.o $(COMMON_OBJ)
	$(CC) $^ $(LDFLAGS) $(extra_libs) -o $@

# uv based servers need libuv
$(BUILD_DIR)/uv_server $(BUILD_DIR)/uv_prime_server: extra_libs := -luv

# concurrency/thread based targets need pthread
$(BUILD_DIR)/thread_server $(BUILD_DIR)/MPMC $(BUILD_DIR)/semaphore: extra_libs := -lpthread

# ---- compile ----
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/utils.o: $(COMMON_DIR)/utils.c $(COMMON_DIR)/utils.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	@echo "Remove linked and compiled files......"
	rm -rf $(BUILD_DIR)
