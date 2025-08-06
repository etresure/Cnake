CC = gcc
LIBS = -lraylib -L. -lglfw3 -framework Cocoa -framework IOKit
SRC = cnake.c
OUT = cnake

all: $(OUT)

$(OUT): $(SRC)
	$(CC) -o $(OUT) $(SRC) $(LIBS)

clean:
	rm -f $(OUT)
