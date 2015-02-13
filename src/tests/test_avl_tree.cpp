
#include <core/avl_tree.h>
#include <random>
#include <functional>
#include <set>
#include "test_helpers.h"

using namespace std;

void test_avl_tree()
{
	auto gen = std::bind(uniform_int_distribution<int>(0, 100000), default_random_engine());

	avl_tree<int, bool> tree;
	std::set<int> correct_tree;

	for (int i = 0; i < 1000; ++i)
	{
		int val = gen();
		//cout << val << std::endl;

		//if (i == 5)
		//	cout << tree;

		auto correct_res = correct_tree.insert(val);
		auto res = tree.insert(make_pair(val, val));
		assert_test(correct_res.second == res.second, "avl tree insertion");
		assert_test(tree.size() == correct_tree.size(), "avl tree size");
		assert_test(tree.validate_height(), "avl tree height validation");

		//std::cout << tree;
	}
	
	for (auto v : correct_tree)
	{
		auto r = tree.find(v);
		assert_test(r != tree.end(), "avl tree find");
	}

	//Ordering
	{
		auto correct_it = correct_tree.begin();
		auto it = tree.begin();
		for (; it != tree.end(); ++it, ++correct_it)
		{
			assert_test(*it == *correct_it, "avl tree ordering");
		}
	}
}

void test_range_map()
{
	/*
	
	0.....4....9.....12......16......19.......23.........
	   0     1     6      2       8       21       7
	*/

	range_map<size_t, size_t> rmap;
	rmap.insert(0, 0);
	rmap.insert(19, 21);
	rmap.insert(23, 7);
	rmap.insert(12, 2);
	rmap.insert(4, 1);
	rmap.insert(9, 6);
	rmap.insert(16, 8);

	rmap.print();

	auto it = rmap.find(0);
	assert_test(it.data() == 0, "range_map find");
	assert_test(rmap.find(1).data() == 0, "range_map find");
	assert_test(rmap.find(3).data() == 0, "range_map find");

	assert_test(rmap.find(4).data() == 1, "range_map find");
	assert_test(rmap.find(5).data() == 1, "range_map find");
	assert_test(rmap.find(8).data() == 1, "range_map find");

	assert_test(rmap.find(17).data() == 8, "range_map find");
	assert_test(rmap.find(18).data() == 8, "range_map find");

	assert_test(rmap.find(19).data() == 21, "range_map find");
	assert_test(rmap.find(20).data() == 21, "range_map find");

	assert_test(rmap.find(23).data() == 7, "range_map find");
	assert_test(rmap.find(100).data() == 7, "range_map find");
}