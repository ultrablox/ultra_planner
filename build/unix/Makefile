include standard_defs.mk

SRC_DIR = ../../src
SRC_CORE_DIR = $(SRC_DIR)/core
SRC_VAL_DIR = ../../dependencies/VAL/src
SRC_TESTS_DIR = ../../src/tests
OUT_DIR = ../../bin/osx

pddl_solver: libcore.a libpddl.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -L. -lcore -lpddl $(SRC_DIR)/planner/main.cpp -o pddl_solver $(INCLUDE_DIRS)
	cp pddl_solver $(OUT_DIR)/pddl_solver

rubik_solver: libcore.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -L. -lcore $(SRC_DIR)/rubik/solver.cpp -o rubik_solver $(INCLUDE_DIRS)
	cp rubik_solver $(OUT_DIR)/rubik_solver

rubik_generator: libcore.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -L. -lcore $(SRC_DIR)/rubik/generator.cpp -o rubik_generator $(INCLUDE_DIRS)

hannoi_solver: libcore.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -L. -lcore $(SRC_DIR)/hannoi/solver.cpp -o hannoi_solver $(INCLUDE_DIRS)
	cp hannoi_solver $(OUT_DIR)/hannoi_solver

puzzle_solver: libcore.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -L. -lcore $(SRC_DIR)/sliding_puzzle/solver.cpp -o puzzle_solver $(INCLUDE_DIRS)
#	$(DSYM) puzzle_solver -o puzzle_solver.dSYM
	cp puzzle_solver $(OUT_DIR)/puzzle_solver

puzzle_generator: libcore.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -L. -lcore $(SRC_DIR)/sliding_puzzle/generator.cpp -o puzzle_generator $(INCLUDE_DIRS)

tests: libtests.a libcore.a
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -L. -ltests -lcore $(SRC_TESTS_DIR)/main.cpp -o tests $(INCLUDE_DIRS)
	cp tests $(OUT_DIR)/tests

libpddl.a:
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $(SRC_DIR)/pddl/planning_task_converter.cpp $(SRC_DIR)/pddl/val_parser.cpp $(SRC_VAL_DIR)/FastEnvironment.cpp $(SRC_VAL_DIR)/SimpleEval.cpp $(SRC_VAL_DIR)/SASActions.cpp $(SRC_VAL_DIR)/ptree.cpp $(SRC_VAL_DIR)/pddl+.cpp $(SRC_VAL_DIR)/TypedAnalyser.cpp $(SRC_VAL_DIR)/typecheck.cpp $(SRC_VAL_DIR)/TimSupport.cpp $(SRC_VAL_DIR)/TIM.cpp $(SRC_VAL_DIR)/FuncAnalysis.cpp $(SRC_VAL_DIR)/instantiation.cpp $(SRC_VAL_DIR)/DebugWriteController.cpp $(SRC_VAL_DIR)/ToFunction.cpp
	ar cr libpddl.a planning_task_converter.o val_parser.o FastEnvironment.o SimpleEval.o SASActions.o ptree.o pddl+.o TypedAnalyser.o typecheck.o TimSupport.o TIM.o FuncAnalysis.o instantiation.o DebugWriteController.o ToFunction.o

libtests.a: 
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $(SRC_TESTS_DIR)/test_helpers.cpp $(SRC_TESTS_DIR)/test_avl_tree.cpp $(SRC_TESTS_DIR)/test_bitset.cpp $(SRC_TESTS_DIR)/test_block_chain.cpp $(SRC_TESTS_DIR)/test_cached_file.cpp $(SRC_TESTS_DIR)/test_complex_vector.cpp $(SRC_TESTS_DIR)/test_merge.cpp $(SRC_TESTS_DIR)/test_puzzle.cpp $(SRC_TESTS_DIR)/test_hannoi_tower.cpp $(SRC_TESTS_DIR)/test_rubik.cpp $(SRC_TESTS_DIR)/test_explicit_graph_search.cpp $(SRC_TESTS_DIR)/test_search.cpp $(SRC_TESTS_DIR)/test_sort.cpp $(SRC_TESTS_DIR)/test_multivar_systems.cpp $(SRC_TESTS_DIR)/test_compressed_stream.cpp $(SRC_TESTS_DIR)/test_external_containers.cpp
	ar cr libtests.a test_helpers.o test_block_chain.o test_avl_tree.o test_bitset.o test_block_chain.o test_cached_file.o test_complex_vector.o test_merge.o test_puzzle.o test_hannoi_tower.o test_rubik.o test_search.o test_sort.o test_explicit_graph_search.o test_multivar_systems.o test_compressed_stream.o test_external_containers.o

libcore.a:
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $(SRC_CORE_DIR)/utils/helpers.cpp $(SRC_CORE_DIR)/UExternalMemoryController.cpp $(SRC_CORE_DIR)/bit_container.cpp $(SRC_CORE_DIR)/masked_bit_vector.cpp $(SRC_CORE_DIR)/UError.cpp $(SRC_CORE_DIR)/numeric_expression.cpp
	ar cr libcore.a helpers.o bit_container.o UExternalMemoryController.o bit_container.o masked_bit_vector.o UError.o numeric_expression.o

clean:
	rm *.o
	rm *.a
	
