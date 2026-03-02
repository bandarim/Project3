# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Target executable name
TARGET = loadbalancer

# Source files
SRC = src/main.cpp \
      src/IPFirewall.cpp \
      src/RequestQueue.cpp \
      src/Config.cpp \
      src/Logger.cpp \
      src/WebServer.cpp \
      src/LoadBalancer.cpp \
      src/Switch.cpp

# Object files (replace .cpp with .o)
OBJ = $(SRC:.cpp=.o)

# Default target
all: $(TARGET)

# Link object files to create executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

# Compile source files into object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(TARGET) src/*.o