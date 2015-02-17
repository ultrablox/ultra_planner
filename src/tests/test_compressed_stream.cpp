
#include <core/compressed_stream.h>
#include <vector>
#include <random>
#include <functional>
#include "test_helpers.h"

using namespace std;

void test_compressed_stream()
{
	std::vector<unsigned char> data(100, 0);

	{
		compressed_stream wstrm(data.data());
		wstrm.write(5, 3);
		wstrm.write(6, 3);

		compressed_stream rstrm(data.data());

		assert_test(rstrm.read(3) == 5, "Compressed stream simple read");
		assert_test(rstrm.read(3) == 6, "Compressed stream simple read");
	}

	{
		//memset(data.data(), 0, data.size());
		compressed_stream wstrm(data.data());
		wstrm.write(28, 5);
		wstrm.write(22, 5);

		compressed_stream rstrm(data.data());

		assert_test(rstrm.read(5) == 28, "Compressed stream simple read");
		assert_test(rstrm.read(5) == 22, "Compressed stream simple read");
	}

	{
		auto gen = std::bind(uniform_int_distribution<int>(0, 31), default_random_engine());

		std::vector<int> correct_vals, read_vals;
		for (int i = 0; i < 100; ++i)
			correct_vals.push_back(gen());

		read_vals.resize(correct_vals.size());

		compressed_stream wstrm(data.data());
		wstrm.write(correct_vals.begin(), correct_vals.end(), 5);

		compressed_stream rstrm(data.data());
		rstrm.read(read_vals.begin(), read_vals.end(), 5);

		assert_test(correct_vals == read_vals, "Compressed stream mass read");
	}
}
