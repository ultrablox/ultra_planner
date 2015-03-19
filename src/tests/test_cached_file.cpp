
#include <core/io/cached_file.h>
#include <random>
#include "test_helpers.h"

struct sample_block_t
{
	sample_block_t(int _val = 0)
		:val(_val)
	{
	}

	int val;
	char data[4096 - sizeof(int)];
};

bool operator==(const sample_block_t & b1, const sample_block_t & b2)
{
	return b1.val == b2.val;
}

void test_basic_io()
{
	std::vector<sample_block_t> correct_data;

	auto gen = std::bind(uniform_int_distribution<int>(0, 999), default_random_engine());
	for (int i = 0; i < 300; ++i)
	{
		sample_block_t block;
		block.val = gen();
		correct_data.push_back(block);
	}

	cached_file<sample_block_t, 10> test_file("test_file.dat");
	test_file.open("test_file.dat");

	//Test push_back operator
	for (int i = 0; i < correct_data.size(); ++i)
		test_file.push_back(correct_data[i]);

	for (int i = 0; i < correct_data.size(); ++i)
		assert_test(correct_data[i] == test_file[i], "Cached file push_back");

	//Assign operator
	test_file[4] = sample_block_t(999);
	assert_test(test_file[4] == sample_block_t(999), "Assign operator");

	//Test random write
	std::random_shuffle(correct_data.begin(), correct_data.end());
	for (int i = 0; i < correct_data.size(); ++i)
		test_file[i] = correct_data[i];

	//Test random read
	cout << "================Testing random read=============" << std::endl;
	for (int i = 0; i < correct_data.size(); ++i)
		assert_test(test_file[i] == correct_data[i], "Cached file random read");
}

void test_large_files()
{
	cached_file<sample_block_t, 1> test_file("test_file.dat");
	test_file.open("test_file.dat");

	std::unordered_map<int, sample_block_t> correct_map;

	size_t first_large_index = 1024ULL * 1024;

	for (int i = 0; i < 200; ++i)
	{
		sample_block_t block;
		block.val = 1000 + i;

		test_file.load(first_large_index + i, block);
		correct_map.insert(make_pair(first_large_index + i, block));
	}

	for (auto & el : correct_map)
		assert_test(test_file[el.first] == el.second, "Large Cached file random read");
}

void test_cached_file()
{
	test_basic_io();
	test_large_files();
}
