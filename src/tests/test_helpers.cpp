
#include "test_helpers.h"
#include <iostream>
#include <random>
#include <functional>

using namespace std;

void assert_test(bool val, const std::string & description)
{
	if(!val)
		cout << "Test '" << description << "' failed\n";
}

void fill_vector(std::vector<int> & vec, size_t num_of_elements, int min_val, int max_val)
{
	vec.clear();

	auto gen = std::bind(uniform_int_distribution<int>(min_val, max_val), default_random_engine());
	for (size_t i = 0; i < num_of_elements; ++i)
		vec.push_back(gen());
}
