
#include "test_sort.h"
#include <core/algorithm/sort.h>
#include <vector>
#include <random>
#include <functional>
#include <algorithm>
#include <iostream>
#include <utility>
#include <chrono>
#include <fstream>
#include <string>

using namespace UltraCore;
using namespace std;
using namespace std::chrono;
/*
template<class S>
class SortTester
{
	void operator()()
	{
		
	}
};*/

void fill_vector(std::vector<int> & vec, size_t num_of_elements)
{
	vec.clear();

	auto gen = std::bind(uniform_int_distribution<int>(0, 3*num_of_elements), default_random_engine());
	for(size_t i = 0; i < num_of_elements; ++i)
		vec.push_back(gen());
}

template<typename T>
struct SortTesterBase
{
	typedef T DataType;
	typedef std::pair<size_t, double> Measure;
	typedef std::vector<T> TestData;

	void operator()(const std::string & sorting_name)
	{
		vector<Measure> performance_data;

		TestData arr;

		bool test_failed = false;
		for(size_t n = 7; n < 1e5; n *= 2ULL)
		{
			fill_vector(arr, n);

			auto sorted_copy = arr;

			auto cmp = std::less<DataType>();
			std::sort(sorted_copy.begin(), sorted_copy.end(), cmp);

			auto start_tp = high_resolution_clock::now();
			
			trySort(arr);

			auto end_tp = high_resolution_clock::now();

			if(sorted_copy != arr)
			{
				cout << "'" << sorting_name << " sort' test failed.\n";
				test_failed = true;
				break;
			}
			else
			{
				performance_data.push_back(make_pair(n, duration_cast<nanoseconds>(end_tp - start_tp).count() * 1e-9));
				cout << "Done testing for " << n << " items.\n";
			}
		}

		if(!test_failed)
		{
			ofstream ofs(sorting_name + string("sort.txt"));
			for(auto & m : performance_data)
				ofs << m.first << "\t" << m.second << "\n";
			ofs.close();
		}
	}

	virtual void trySort(TestData & arr) const = 0;
};

template<typename DataType, template<typename T, typename A> class S>
struct SortTester : public SortTesterBase<DataType>
{
	using TestData = typename SortTesterBase<DataType>::TestData;

	virtual void trySort(TestData & arr) const override
	{
		ultra_sort<S>(arr, std::less<DataType>());
	}
	
};

template<typename DataType, template<typename T> class S>
struct IntSortTester : public SortTesterBase<DataType>
{
	using TestData = typename SortTesterBase<DataType>::TestData;
	
	virtual void trySort(TestData & arr) const override
	{
		ultra_sort<S>(arr);
	}
	
};

void test_sorting()
{
	//SortTester<int, insertion_sorter>()("insertion");
	//SortTester<int, MergeSorter>()("merge");
	//SortTester<int, HeapSorter>()("heap");
	//IntSortTester<int, CountSorter>()("count");
	IntSortTester<int, RadixSorter>()("radix");
}
