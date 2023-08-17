

CXX := clang++

SRC_DIRS := .
BUILD_DIR := ../build

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c')
HDRS := $(shell find $(SRC_DIRS) -name '*.hpp' -or -name '*.h')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CXXFLAGS := -std=c++17 -Wall -Wextra -ggdb -g -O0
CPPFLAGS ?= $(INC_FLAGS) -I$(DLL_DIR) -MMD -MP
LIBS := -lm -ldl -lX11
LDFLAGS := -rdynamic

TARGET ?= memory_tracker


$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LIBS)

$(BUILD_DIR)/%.c.o: %.c $(HDRS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.cpp.o: %.cpp $(HDRS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY = clean

clean:
	@echo "removing object files and executables..."
	@rm -f $(BUILD_DIR)/$(OBJS) $(BUILD_DIR)/*.cpp.d $(BUILD_DIR)/*.c.d $(BUILD_DIR)/$(TARGET)


-include $(DEPS)
