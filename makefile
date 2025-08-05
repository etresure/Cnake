CC = gcc
LIBS = -lraylib -L. -lglfw3 -framework Cocoa -framework IOKit
SRC = snake.c
OUT = snake

all: $(OUT)

$(OUT): $(SRC)
	$(CC) -o $(OUT) $(SRC) $(LIBS)

clean:
	rm -f $(OUT)
