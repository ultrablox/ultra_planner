include standard_defs.mk

SRC_DIR = ../../src
SRC_CORE_DIR = $(SRC_DIR)/core
OUT_DIR = ../../bin/osx

puzzle_solver: core.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) UExternalMemoryController.o helpers.o $(SRC_DIR)/sliding_puzzle/main.cpp -o $(OUT_DIR)/puzzle_solver $(INCLUDE_DIRS)

core.o:
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $(SRC_CORE_DIR)/utils/helpers.cpp $(SRC_CORE_DIR)/UExternalMemoryController.cpp
