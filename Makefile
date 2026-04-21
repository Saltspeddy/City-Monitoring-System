CC = gcc
CFLAGS = -Wall -Wextra -I Core/Inc/
 
SRC = Core/Src/city_manager.c
OUT = city_manager
 
all: $(OUT)
 
$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)
 
clean:
	rm -f $(OUT)