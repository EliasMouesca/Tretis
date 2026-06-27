# FLAGS
CC      := gcc
CFLAGS  := -ggdb -Wall -Wno-stringop-truncation -Wextra -Wmisleading-indentation -O2 -Iinclude $(shell pkg-config --cflags sdl3)
LDFLAGS := $(shell pkg-config --libs sdl3 sdl3-ttf) -lm
VALGRINDFLAGS := --tool=memcheck --leak-check=yes --track-origins=yes --num-callers=12 -s --quiet --show-leak-kinds=definite,indirect --errors-for-leak-kinds=definite,indirect

PWD := $(shell pwd)
SRC_DIR := src
OBJ_DIR := obj
TARGET  := main
PREFIX  ?= /usr

SRCS := $(shell find $(SRC_DIR) -name '*.c')
MODULES_SRCS := $(filter-out %_test.c $(SRC_DIR)/main.c,$(SRCS))
MODULES := $(sort $(patsubst $(SRC_DIR)/%/,%,$(dir $(MODULES_SRCS))))
MAIN_SRC := $(SRC_DIR)/main.c

MODULES_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(MODULES_SRCS))
MAIN_OBJ := $(OBJ_DIR)/main.o

all: $(TARGET)

$(TARGET): $(MODULES_OBJS) $(MAIN_OBJ)
	$(CC) $(MODULES_OBJS) $(MAIN_OBJ) $(LDFLAGS) -o $@ && printf "\nCompilation successful!\n\n"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

TEST_SRCS := $(filter %_test.c,$(SRCS))
TEST_OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(TEST_SRCS))
TEST_MODULES := $(sort $(patsubst $(SRC_DIR)/%/,%,$(dir $(TEST_SRCS))))
TESTS := $(patsubst %,test-%,$(TEST_MODULES))

test: $(TESTS)
	@if [ -z "$(TESTS)" ]; then \
		printf "\033[33mNo tests found.\033[0m\n\n"; \
	else \
		printf "\033[1;32mAll tests passed!\033[0m\n\n"; \
	fi

test-run: $(TARGET)
	valgrind $(VALGRINDFLAGS) $(PWD)/$< --block-size 7

test-stress: $(TARGET)
	$(PWD)/$< --block-size 7 --fullscreen --delay 100 --fps 60 --party

clean:
	rm -rf $(OBJ_DIR) $(TARGET) test test-*

install: $(TARGET)
	install -Dm755 $(TARGET) $(DESTDIR)$(PREFIX)/bin/tretis
	install -Dm644 fonts/SpaceMono-Regular.ttf $(DESTDIR)$(PREFIX)/share/tretis/fonts/SpaceMono-Regular.ttf

.PHONY: all clean install test test-*


# Magia rara porque make no deja usar dos veces la wildcard (%)
define build_test_rule
test-$1: $(MODULES_OBJS) $(OBJ_DIR)/$1/$1_test.o
	@$(CC) $$^ $(LDFLAGS) -o $$@
	@printf "\033[90m\n=== $1 test ===\033[0m\n"
	@valgrind $(VALGRINDFLAGS) $(PWD)/$$@ && printf "\033[32m\nPassed!\n\033[0m"
	@printf "\033[90m===============\033[0m\n\n"
	@rm $$@
endef
$(foreach f,$(TEST_MODULES),$(eval $(call build_test_rule,$f)))
