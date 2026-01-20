CC = gcc
CFLAGS = -Wall -Wextra -std=c11 `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`

SRC = ui/main.c core/canvas.c core/brush.c core/tool.c core/theme.c core/tool_factory.c core/fill.c core/history.c
OUT = paint

all:
	$(CC) $(SRC) $(CFLAGS) -o $(OUT) $(LDFLAGS)

clean:
	rm -f $(OUT)



