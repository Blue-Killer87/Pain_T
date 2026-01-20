CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`

SOURCES = core/canvas.cpp core/brush.cpp core/fill.cpp core/history.cpp core/image_tool.cpp ui/main.cpp 
TARGET = paint

all:
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)
