
#include "test_helpers.h"
#include <core/UBitset.h>

void test_bitset()
{
	string res;
	//Check that clear works fine
	UBitset bs(10);
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
		UBitset b1(10), b2(10), b3(10);
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
		UBitset b1(10), b2(10);
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
		UBitset b1(10), b2(10);
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
		UBitset b1(10);
		b1.clear();

		b1[4] = true;
		b1[5] = true;
		b1[6] = true;
		
		auto b2 = ~b1;

		assert_test(b2.toString() == "1111000111", "operator~");
	}

	//Special []
	{
		UBitset b1(306);
		b1.clear();
		//b1.print();
		b1[226] = true;
		bool t = b1[194];
		assert_test(!t, "[] on big indices (>32)");
		//b1.print();
	}

	//Set masked
	{
		UMaskedBitVector bv(10);
		bv.clear();

		UBitset b1(10);
		b1.clear();
		b1[3] = true;
		b1[4] = true;
		b1[5] = true;
		b1[6] = true;

		bv.set(1, true);
		bv.set(2, true);
		bv.set(4, false);
		bv.set(5, false);

		b1.setMasked(bv.state, bv.mask);
		res = b1.toString();
		assert_test(res == "0111001000", "setMasked");
	}

	//Equal masked
	{
		UMaskedBitVector bv(10);
		bv.clear();
		bv.set(1, false);
		bv.set(2, true);
		bv.set(5, true);
		bv.set(6, false);
		bv.set(9, false);

		UBitset b1(10);
		b1.clear();
		b1[2] = true;
		b1[5] = true;
		b1[7] = true;		

		assert_test(b1.equalMasked(bv.state, bv.mask), "setMasked positive");

		UBitset b2(10);
		b2.clear();
		b2[2] = true;
		b2[4] = true;
		b2[6] = true;		

		bool r = b2.equalMasked(bv.state, bv.mask);
		assert_test(!r, "setMasked negative");
	}

	//Equal masked
	{
		UMaskedBitVector bv(10);
		bv.clear();
		bv.set(1, false);
		bv.set(2, true);
		bv.set(5, true);
		bv.set(6, false);
		bv.set(9, false);

		UBitset b1(10);
		b1.clear();
		b1[2] = true;
		b1[5] = true;
		b1[7] = true;		

		assert_test(b1.equalCountMasked(bv.state, bv.mask) == 5, "equalCountMasked positive");
	}

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


		UBitset b1(36);
		b1.clear();

		for(auto i : correct_vec)
			b1[i] = true;
		
		for(auto it = b1.pbegin(); it != b1.pend(); ++it)
		{
			vals.push_back(it.index());
		}

		assert_test(correct_vec == vals, "positive bits iteration");

	}
}
