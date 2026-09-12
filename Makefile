# printf-toto — GNU Make build (C11, -Werror). See c_version.txt / .cursor/rules.

CC := $(shell command -v gcc >/dev/null 2>&1 && echo gcc || echo clang)

CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Werror \
	-D_POSIX_C_SOURCE=200809L -Iinclude -O2
	
CFLAGS_DEBUG := -std=c11 -Wall -Wextra -Wpedantic -Werror \
	-D_POSIX_C_SOURCE=200809L -Iinclude \
	-g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer

BUILD_DIR := build
TEST_BUILD_DIR := build/tests

TARGET := $(BUILD_DIR)/printf-toto
TARGET_DEBUG := $(BUILD_DIR)/printf-toto-debug
TEST_CORE := $(TEST_BUILD_DIR)/test_core

SRCS := src/main.c \
	src/printf_toto_emit.c \
	src/printf_toto_format.c \
	src/printf_toto_write.c \
	src/printf_toto_cli.c

OBJS := $(BUILD_DIR)/main.o \
	$(BUILD_DIR)/printf_toto_emit.o \
	$(BUILD_DIR)/printf_toto_format.o \
	$(BUILD_DIR)/printf_toto_write.o \
	$(BUILD_DIR)/printf_toto_cli.o

TEST_SRCS := tests/test_runner.c \
	tests/test_escape_basic.c \
	tests/test_conv_percent.c \
	tests/test_conv_s.c \
	tests/test_conv_d.c \
	tests/test_arg_consume.c

TEST_OBJS := $(TEST_BUILD_DIR)/test_runner.o \
	$(TEST_BUILD_DIR)/test_escape_basic.o \
	$(TEST_BUILD_DIR)/test_conv_percent.o \
	$(TEST_BUILD_DIR)/test_conv_s.o \
	$(TEST_BUILD_DIR)/test_conv_d.o \
	$(TEST_BUILD_DIR)/test_arg_consume.o

# Format helpers + deps (no main).
TEST_LIB_OBJS := $(BUILD_DIR)/printf_toto_format.o \
	$(BUILD_DIR)/printf_toto_emit.o \
	$(BUILD_DIR)/printf_toto_write.o \
	$(BUILD_DIR)/printf_toto_cli.o

.PHONY: all debug test clean install

all: $(TARGET)

$(BUILD_DIR) $(TEST_BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

debug: $(TARGET_DEBUG)

$(TARGET_DEBUG): $(SRCS) | $(BUILD_DIR)
	$(CC) $(CFLAGS_DEBUG) -o $@ $(SRCS)

$(TEST_BUILD_DIR)/%.o: tests/%.c | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_CORE): $(TEST_OBJS) $(TEST_LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(TEST_LIB_OBJS)

test: $(TEST_CORE)
	./$(TEST_CORE)

clean:
	rm -f $(BUILD_DIR)/*.o $(BUILD_DIR)/printf-toto $(BUILD_DIR)/printf-toto.exe \
		$(BUILD_DIR)/printf-toto-debug $(BUILD_DIR)/printf-toto-debug.exe \
		$(TEST_BUILD_DIR)/*.o $(TEST_BUILD_DIR)/test_core $(TEST_BUILD_DIR)/test_core.exe

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/printf-toto
