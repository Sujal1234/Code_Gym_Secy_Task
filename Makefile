CXX = g++
CXXFLAGS = -Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor \
		   -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion \
		   -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference \
		   -Wuseless-cast -Wdouble-promotion -Wformat=2 -pedantic -g -std=c++20
DEPFLAGS = -MMD -MP

TARGET = engine

SRCS = $(wildcard src/*.cpp)
OBJS = $(patsubst src/%.cpp, obj/%.o, $(SRCS))
INCLUDES = -Iinclude
DEPS = $(patsubst obj/%.o, obj/%.d, $(OBJS))

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

obj/%.o: src/%.cpp
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(INCLUDES) -c $< -o $@

# Include dependency files
# Automatically recompile .cpp files if included .h files change
-include $(DEPS)

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

.PHONY: clean