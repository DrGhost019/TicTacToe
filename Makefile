CXX       := g++
CXXFLAGS  := -std=c++20 -O2 -Wall -Wextra -Wpedantic -Isrc
BUILD_DIR := build

QT_CFLAGS := $(shell pkg-config --cflags Qt6Widgets Qt6Gui Qt6Core)
QT_LIBS   := $(shell pkg-config --libs Qt6Widgets Qt6Gui Qt6Core)
QT_LIBEXEC:= $(shell pkg-config --variable=libexecdir Qt6Core)
MOC       := $(firstword $(wildcard $(QT_LIBEXEC)/moc /usr/lib/qt6/moc /usr/libexec/qt6/moc) $(shell which moc-qt6 moc 2>/dev/null))

ENGINE_SRCS := src/engine/Board.cpp src/engine/AI.cpp src/engine/GameSession.cpp
CLI_SRCS    := src/cli/ConsoleRunner.cpp
GUI_SRCS    := src/gui/BoardWidget.cpp src/gui/MainWindow.cpp
MOC_HDRS    := src/gui/BoardWidget.hpp src/gui/MainWindow.hpp

MOC_SRCS    := $(patsubst src/gui/%.hpp,$(BUILD_DIR)/moc_%.cpp,$(MOC_HDRS))
ENGINE_OBJS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(ENGINE_SRCS))
CLI_OBJS    := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(CLI_SRCS))
GUI_OBJS    := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(GUI_SRCS))
MOC_OBJS    := $(patsubst $(BUILD_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(MOC_SRCS))
MAIN_OBJ    := $(BUILD_DIR)/main.o

TARGET_GUI  := TicTacToe
TARGET_CLI  := TicTacToe-cli
TARGET_TEST := $(BUILD_DIR)/test_engine

.PHONY: all gui cli test run clean

all: gui

gui: $(TARGET_GUI)

$(BUILD_DIR)/moc_%.cpp: src/gui/%.hpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(MOC) -Isrc $< -o $@

$(BUILD_DIR)/engine/%.o: src/engine/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/cli/%.o: src/cli/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/gui/%.o: src/gui/%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -fPIC $(QT_CFLAGS) -c $< -o $@

$(BUILD_DIR)/moc_%.o: $(BUILD_DIR)/moc_%.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -fPIC $(QT_CFLAGS) -c $< -o $@

$(MAIN_OBJ): src/main.cpp | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -fPIC $(QT_CFLAGS) -c $< -o $@

$(TARGET_GUI): $(ENGINE_OBJS) $(CLI_OBJS) $(GUI_OBJS) $(MOC_OBJS) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ $(QT_LIBS) -o $@

cli: $(ENGINE_OBJS) $(CLI_OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -DTTT_CLI_ONLY $(ENGINE_OBJS) $(CLI_OBJS) src/main.cpp -o $(TARGET_CLI)

test: $(ENGINE_OBJS) tests/test_engine.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(ENGINE_OBJS) tests/test_engine.cpp -o $(TARGET_TEST)
	./$(TARGET_TEST)

run: $(TARGET_GUI)
	./$(TARGET_GUI)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET_GUI) $(TARGET_CLI)
