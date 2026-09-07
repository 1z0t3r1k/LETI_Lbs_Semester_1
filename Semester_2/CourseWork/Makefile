CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11

TARGET = cw

SRC = main.c cli.c image.c operations.c png_io.c
HDR = cli.h errors.h image.h operations.h png_io.h

LDLIBS = -lpng

FORMAT_FILES = $(SRC) $(HDR)

.PHONY: all clean format format-check

all: $(TARGET)

$(TARGET): $(SRC) $(HDR)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

format:
	clang-format -style=file -i $(FORMAT_FILES)

format-check:
	clang-format -style=file --dry-run --Werror $(FORMAT_FILES)

clean:
	rm -f $(TARGET) $(TARGET).exe