
#include "test_helpers.h"
#include <core/bit_container.h>
#include <core/masked_bit_vector.h>
#include <random>

using namespace std;

void test_bitset()
{
	cout << "Testing bitsets..." << std::endl;
	
	string res;
	//Check that clear works fine
	bit_vector bs(10);
	bs.clear();
	/*assert_test("0000000000" == bs.toString(), "Clear");*/

	//Check setValue
	bs.setValues(true);
	res = bs.toString();
	assert_test(res == "1111111111", "setValues(true)");

	//setValue(0)
	bs.setValues(false);
	res = bs.toString();
	assert_test(res == "0000000000", "setValues(false)");

	//operator[i] set
	bs[2] = true;
	bs[5] = true;
	bs[7] = true;
	res = bs.toString();
	assert_test(res == "0010010100", "operator[i] set");

	//Check random access
	assert_test(bs[2], "operator[i] get");
	assert_test(bs[5], "operator[i] get");
	assert_test(bs[7], "operator[i] get");
	assert_test(!bs[1], "operator[i] get");
	assert_test(!bs[3], "operator[i] get");
	assert_test(!bs[8], "operator[i] get");
	
	//Check operator ==
	{
		bit_vector b1(10), b2(10), b3(10);
		b1.clear();
		b2.clear();
		b3.clear();

		b1[2] = true;
		b2[2] = true;
		b3[2] = false;

		b1[5] = true;
		b2[5] = true;
		b3[5] = true;

		b1[7] = true;
		b2[7] = true;
		b3[7] = true;

		assert_test(b1 == b2, "operator ==");
		assert_test(!(b2 == b3), "operator ==");
		assert_test(!(b1 == b3), "operator ==");
	}

	

	//Operator &
	{
		bit_vector b1(10), b2(10);
		b1.clear();
		b2.clear();

		b1[4] = true;
		b1[5] = true;
		b1[6] = true;
		b1[7] = true;
		b1[8] = true;

		b2[0] = true;
		b2[1] = true;
		b2[6] = true;
		b2[7] = true;
		b2[8] = true;
		b2[9] = true;

		auto b3 = b1 & b2;
		assert_test(b3.toString() == "0000001110", "operator&");
		//b3.print();
	}

	//Operator ^
	{
		bit_vector b1(10), b2(10);
		b1.clear();
		b2.clear();

		b1[4] = true;
		b1[5] = true;
		b1[6] = true;
		
		b2[0] = true;
		b2[1] = true;
		b2[6] = true;

		auto b3 = b1 ^ b2;
		assert_test(b3.toString() == "1100110000", "operator^");
	}

	//Operator ~
	{
		bit_vector b1(10);
		b1.clear();

		b1[4] = true;
		b1[5] = true;
		b1[6] = true;
		
		auto b2 = ~b1;

		assert_test(b2.toString() == "1111000111", "operator~");
	}

	//Special []
	{
		bit_vector b1(306);
		b1.clear();
		//b1.print();
		b1[226] = true;
		bool t = b1[194];
		assert_test(!t, "[] on big indices (>32)");
		//b1.print();
	}

	//Set masked
	{
		masked_bit_vector bv(10);
		bv.clear();

		bit_vector b1(10);
		b1.clear();
		b1[3] = true;
		b1[4] = true;
		b1[5] = true;
		b1[6] = true;

		bv.set(1, true);
		bv.set(2, true);
		bv.set(4, false);
		bv.set(5, false);

		b1.set_masked(bv.value, bv.mask);
		res = b1.toString();
		assert_test(res == "0111001000", "setMasked");
	}

	//Equal masked
	{
		masked_bit_vector bv(10);
		bv.clear();
		bv.set(1, false);
		bv.set(2, true);
		bv.set(5, true);
		bv.set(6, false);
		bv.set(9, false);

		bit_vector b1(10);
		b1.clear();
		b1[2] = true;
		b1[5] = true;
		b1[7] = true;		

		assert_test(b1.equal_masked(bv.value, bv.mask), "setMasked positive");

		bit_vector b2(10);
		b2.clear();
		b2[2] = true;
		b2[4] = true;
		b2[6] = true;		

		bool r = b2.equal_masked(bv.value, bv.mask);
		assert_test(!r, "setMasked negative");
	}

	//Equal masked (fail in elevators-01)
	{
		masked_bit_vector mv(86);
		mv.set(64 + 3, true);
		mv.set(64 + 8, true);
		mv.set(64 + 15, true);

		bit_vector bv(86);
		for (int i = 0; i < 9; ++i)
			bv[i] = true;
		

		assert_test(!bv.equal_masked(mv.value, mv.mask), "equalMasked");
	}

	//Equal masked count
/*	{
		masked_bit_vector bv(10);
		bv.clear();
		bv.set(1, false);
		bv.set(2, true);
		bv.set(5, true);
		bv.set(6, false);
		bv.set(9, false);

		bit_vector b1(10);
		b1.clear();
		b1[2] = true;
		b1[5] = true;
		b1[7] = true;		

		assert_test(b1.equalCountMasked(bv.value, bv.mask) == 5, "equalCountMasked positive");
	}*/

//Positive bits iteration
	{
		vector<int> vals, correct_vec;

		correct_vec.push_back(2);
		correct_vec.push_back(5);
		correct_vec.push_back(7);
		correct_vec.push_back(19);
		correct_vec.push_back(29);
		correct_vec.push_back(36);
		correct_vec.push_back(49);
		correct_vec.push_back(63);


		bit_vector b1(67);
		b1.clear();

		for (auto i : correct_vec)
			b1[i] = true;

		b1.for_each_true([&](int idx){
			vals.push_back(idx);
		});

		assert_test(correct_vec == vals, "positive bits iteration");

	}

	//For-each true
	{
		vector<int> vals, correct_vec;

		correct_vec.push_back(2);
		correct_vec.push_back(5);
		correct_vec.push_back(7);
		correct_vec.push_back(19);
		correct_vec.push_back(29);
		correct_vec.push_back(36);
		correct_vec.push_back(49);
		correct_vec.push_back(63);


		bit_vector b1(67);
		b1.clear();

		for (auto i : correct_vec)
			b1[i] = true;

		b1.for_each_true([&](int idx){
			vals.push_back(idx);
		});

		assert_test(correct_vec == vals, "bit_vector: foreach true");
	}


	//Remove indices (by one)
	{
		auto gen = std::bind(uniform_int_distribution<int>(0, 300), default_random_engine());
		bit_vector bv(300, false);
		std::vector<bool> correct_vec(300, false);

		bool fail = false;

		bv.set(0, true);
		correct_vec[0] = true;
		if (!(bv == correct_vec))
		{
			cout << "Fail" << std::endl;
		}

		for (int i = 0; i < 200; ++i)
		{
			int index = gen();

			if (index == 160)
			{
				int h = 9;
			}

			bv.set(index, true);
			correct_vec[index] = true;

			if (!(bv == correct_vec))
			{
				cout << "Fail" << std::endl;
			}
		}

		assert_test(bv == correct_vec, "bit_vector: assignment");

		for (int i = 0; i < 100; ++i)
		{
			int index = gen() % (correct_vec.size() - 1);
			correct_vec.erase(correct_vec.begin() + index);

			std::vector<int> indices(1, index);
			bv.remove_indices(indices.begin(), indices.end());
			
			fail = fail && !(bv == correct_vec);
			if (!fail)
			{
				break;
			}
		}

		assert_test(bv == correct_vec, "bit_vector: removing by one");
	}

	//Remove indices (mass)
	{
		auto gen = std::bind(uniform_int_distribution<int>(0, 299), default_random_engine());
		bit_vector bv(300, false);
		std::vector<bool> correct_vec(300, false);

		for (int i = 0; i < 200; ++i)
		{
			int index = gen();
			bv.set(index, true);
			correct_vec[index] = true;
		}

		std::vector<int> indices;

		for (int i = 0; i < 100; ++i)
			indices.push_back(gen());

		std::sort(indices.begin(), indices.end(), std::greater<int>());
		auto last_it = std::unique(indices.begin(), indices.end());
		indices.erase(last_it, indices.end());

		for (int idx : indices)
			correct_vec.erase(correct_vec.begin() + idx);

		std::random_shuffle(indices.begin(), indices.end());
		bv.remove_indices(indices.begin(), indices.end());

		assert_test(bv == correct_vec, "bit_vector: removing mass");
	}
}
