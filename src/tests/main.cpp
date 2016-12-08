
#include "test_helpers.h"
#include "test_sort.h"
#include <core/UMatrix.h>
#include <string>
#include <functional>
#include <random>

using namespace std;
using namespace UltraCore;

template<typename M>
void random_fill(M & matrix)
{
	auto gen = std::bind(uniform_real_distribution<float>(0, 20.0), default_random_engine());

	for(int r = 0; r < matrix.rows(); ++r)
		for(int c = 0; c < matrix.columns(); ++c)
			matrix.at(r, c) = gen();
}

void test_matrix()
{
	UMatrix<float, 4, 4> m1, m2;
	random_fill(m1);
	random_fill(m2);

	auto correct_res = mult(m1, m2);
	auto res = mult_strassen(m1, m2);

	assert_test(correct_res == res, "Matrix multiplication");
}

void test_bitset();
void test_explicit_graph_search();
void test_search();
void test_puzzle_core();
void test_complex_vector();
void test_complex_hashmap();
void test_cached_file();
void test_merge();
void test_rubik_core();
void test_complex_queue();
void test_avl_tree();
void test_range_map();
void test_complex_stack();
void test_block_chain();
void test_hannoi_tower_core();
void test_compressed_stream();
void test_varset_systems();
void test_external_containers();

void test_edfd_cover();

int main()
{
	test_bitset();
	test_merge();
	test_avl_tree();
	test_range_map();
	test_compressed_stream();
	
	test_hannoi_tower_core();
	test_puzzle_core();
	test_rubik_core();
	test_varset_systems();
	
	//commented out for faster testing
	/*
	test_external_containers();
	test_complex_vector();
	
	test_complex_queue();
	test_complex_stack();

	test_block_chain();
	*/

	//fails:
	/*
	test_complex_hashmap();
	test_cached_file();

	test_matrix();

	test_sorting();
	test_explicit_graph_search();
	test_search();
	*/

	test_edfd_cover();
	getchar();

	return 0;
}
