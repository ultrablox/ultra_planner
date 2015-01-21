
#include <core/bdd_map.h>
#include <set>
#include <random>
#include "helpers.h"

void test_bdd_map()
{
	auto gen = std::bind(uniform_int_distribution<int>(0, 15), default_random_engine());

	std::set<int> correct_set;

	bdd_map<int> bmap;

	for (int i = 0; i < 5; ++i)
	{
		auto val = gen();

		//if (i == 11)
		//	cout << bmap;

		auto bres = bmap.insert(val, val);
		auto correct_res = correct_set.insert(val);

		//if (i == 11)
		//cout << bmap;

		assert_test(correct_res.second == bres, "bdd_map insertion: " + to_string(i));
	}

	bmap.print_line();
}
