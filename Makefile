CC := gcc

TARGET := tretis

SRC_DIR := src
OBJ_DIR := obj

CFLAGS := -std=c11 \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wshadow \
	-Wconversion \
	-Werror \
	-g3 \
	-O0

CFLAGS += $(shell pkg-config --cflags sdl3)

LDLIBS := $(shell pkg-config --libs sdl3)

SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

.PHONY: all clean run debug

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

debug: all
	gdb ./$(TARGET)

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(TARGET)
