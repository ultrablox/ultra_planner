
#ifndef UltraTest_helpers_h
#define UltraTest_helpers_h

#include <string>
#include <vector>

void assert_test(bool val, const std::string & description);
void fill_vector(std::vector<int> & vec, size_t num_of_elements, int min_val, int max_val);

template<typename It1, typename It2>
bool compare_paths(It1 begin1, It1 end1, It2 begin2, It2 end2)
{
	if (std::distance(begin1, end1) != std::distance(begin2, end2))
		return false;

	while (begin1 != end1)
	{
		if (*begin1 != *begin2)
			return false;

		++begin1;
		++begin2;
	}

	return true;
}

template<typename C1, typename C2>
bool compare_paths(C1 path1, C2 path2)
{
	return compare_paths(path1.begin(), path1.end(), path2.begin(), path2.end());
}

#endif
