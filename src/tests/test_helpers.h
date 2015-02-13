
#ifndef UltraTest_helpers_h
#define UltraTest_helpers_h

#include <string>
#include <vector>

void assert_test(bool val, const std::string & description);
void fill_vector(std::vector<int> & vec, size_t num_of_elements, int min_val, int max_val);

#endif
