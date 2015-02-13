
#include "test_helpers.h"
#include "test_sort.h"
#include <core/algorithm/merge.h>
#include <vector>
#include <algorithm>

using namespace std;

void test_merge()
{
	//Test unique
	{
		vector<int> ext_cont;
		for (int i = 0; i < 10; i += 2)
		for (int j = 0; j < i / 2; ++j)
			ext_cont.push_back(i);

		vector<int> main_cont;
		main_cont.push_back(3);
		main_cont.push_back(4);
		main_cont.push_back(4);
		main_cont.push_back(5);
		main_cont.push_back(5);
		main_cont.push_back(6);
		main_cont.push_back(12);
		main_cont.push_back(14);

		vector<int> xres;
		xres.push_back(3);
		xres.push_back(5);
		xres.push_back(5);
		xres.push_back(12);
		xres.push_back(14);

		auto keyer = [](const int & val){ return val; };
		auto equaler = [](const int v1, const int v2){ return v1 == v2; };
		auto last_it = UltraCore::unique(ext_cont.begin(), ext_cont.end(), main_cont.begin(), main_cont.end(), keyer, keyer, equaler);
		main_cont.erase(last_it, main_cont.end());

		assert_test(xres == main_cont, "unique(2 containers)");
	}

	{
		vector<int> ext_cont;
		ext_cont.push_back(74);

		vector<int> main_cont;
		main_cont.push_back(74);
		main_cont.push_back(74);
		main_cont.push_back(74);
		main_cont.push_back(120);

		vector<int> xres;
		xres.push_back(74);
		xres.push_back(74);
		xres.push_back(74);
		xres.push_back(120);

		auto keyer = [](const int & val){ return val; };
		auto equaler = [](const int v1, const int v2){ return false; };
		auto last_it = UltraCore::unique(ext_cont.begin(), ext_cont.end(), main_cont.begin(), main_cont.end(), keyer, keyer, equaler);
		main_cont.erase(last_it, main_cont.end());

		assert_test(xres == main_cont, "unique(2 containers)");
	}

	//Unique with one container
	{
		vector<int> ext_cont, good_cont;
		for (int i = 0; i < 10; i += 2)
		for (int j = 0; j < i / 2; ++j)
			ext_cont.push_back(i);

		good_cont.push_back(2);
		good_cont.push_back(4);
		good_cont.push_back(6);
		good_cont.push_back(8);

		auto keyer = [](const int & val){ return val; };
		auto equaler = [](const int v1, const int v2){ return v1 == v2; };

		auto last_it = UltraCore::unique(ext_cont.begin(), ext_cont.end(), keyer, equaler);
		ext_cont.erase(last_it, ext_cont.end());

		assert_test(ext_cont == good_cont, "unique(1 container)");
	}

	//Merge
	{
		std::vector<int> src, dest;
		fill_vector(dest, 20, 0, 100);
		fill_vector(src, 10, 0, 100);

		auto cmp = [](int e1, int e2){return e1 < e2; };
		std::sort(src.begin(), src.end(), cmp);
		std::sort(dest.begin(), dest.end(), cmp);

		

		vector<int> correct_res(dest.size() + src.size());
		std::merge(src.begin(), src.end(), dest.begin(), dest.end(), correct_res.begin(), cmp);

		dest.resize(dest.size() + src.size());
		UltraCore::merge(dest.rbegin() + src.size(), dest.rend(), src.rbegin(), src.rend(), dest.rbegin(), [](int e1, int e2){return e1 > e2; });

		assert_test(dest == correct_res, "merge");
	}

	{
		std::vector<int> src, dest;
		src.push_back(74);
		src.push_back(74);
		src.push_back(74);
		src.push_back(54);

		dest.push_back(74);

		vector<int> correct_res;
		correct_res.push_back(74);
		correct_res.push_back(74);
		correct_res.push_back(74);
		correct_res.push_back(74);
		correct_res.push_back(54);


		auto cmp = [](int e1, int e2){return e1 < e2; };

		
		dest.resize(dest.size() + src.size());
		//UltraCore::merge(dest.rbegin(), dest.rbegin() + src.size(), dest.rend(), src.rbegin(), src.rend(), cmp);
		std::merge(dest.rbegin() + src.size(), dest.rend(), src.rbegin(), src.rend(), dest.rbegin(), cmp);


		assert_test(dest == correct_res, "merge");
	}


	//Group end
	{
		std::vector<int> src;
		src.push_back(1);
		src.push_back(2);
		src.push_back(2);
		src.push_back(2);
		src.push_back(3);
		src.push_back(3);
		src.push_back(4);
		src.push_back(4);
		src.push_back(4);
		src.push_back(8);

		{
			auto it1 = UltraCore::find_group_end(src.begin(), src.end(), 2, [](const int val){return val; });
			assert_test(std::distance(src.begin(), it1) == 1, "find_group_end");
		}

		{
			auto it1 = UltraCore::find_group_end(src.begin(), src.end(), 3, [](const int val){return val; });
			assert_test(std::distance(src.begin(), it1) == 1, "find_group_end");
		}

		{
			auto it1 = UltraCore::find_group_end(src.begin(), src.end(), 4, [](const int val){return val; });
			assert_test(std::distance(src.begin(), it1) == 4, "find_group_end");
		}
	}
	
}
