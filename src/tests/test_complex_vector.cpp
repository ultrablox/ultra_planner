
#include "test_helpers.h"
#include <core/containers/complex_vector.h>
#include <core/containers/complex_queue.h>
#include <core/containers/complex_stack.h>
#include <core/complex_hashset/generator.h>
#include <random>
#include <chrono>
#include <unordered_set>
#include <array>
#include <queue>

using namespace std;
using namespace std::chrono;

struct complex_element
{
	complex_element(int val = 0)
	{
		m_data.push_back(val / 100);
		val = val % 100;

		m_data.push_back(val / 10);
		m_data.push_back(val % 10);
	}

	complex_element(int x, int y, int z)
	{
		m_data.push_back(x);
		m_data.push_back(y);
		m_data.push_back(z);
	}

	int val() const
	{
		return m_data[0]*100 + m_data[1] * 10 + m_data[2];
	}

	std::vector<int> m_data;
};

class complex_element_streamer : public streamer_base
{
public:
	complex_element_streamer()
		:streamer_base(3*sizeof(int))
	{
	}

	void serialize(void * dst, const complex_element & val) const
	{
		memcpy(dst, &val.m_data[0], m_serializedSize);
	}

	void deserialize(const void * src, complex_element & val) const
	{
		val.m_data.resize(3);
		memcpy(&val.m_data[0], src, m_serializedSize);
	}
};

bool operator==(const complex_element & lhs, const complex_element & rhs)
{
	return lhs.m_data == rhs.m_data;
}

namespace std
{
	bool operator<(const complex_element & lhs, const complex_element & rhs)
	{
		return lhs.val() < rhs.val();
	}

	template<>
	class hash<complex_element>
	{
	public:
		size_t operator()(const complex_element & el) const
		{
			//return el.val() % 100;
			return el.val();
		}
	};
};


void test_complex_vector()
{
	complex_element_streamer stremer;
	complex_vector<complex_element, complex_element_streamer> vec(stremer);

	vec.push_back(complex_element(0, 0, 1));
	vec.push_back(complex_element(0, 1, 0));
	vec.push_back(complex_element(1, 0, 0));

	assert_test(vec[0] == complex_element(0, 0, 1), "Complex vector []");
	assert_test(vec[1] == complex_element(0, 1, 0), "Complex vector []");
	assert_test(vec[2] == complex_element(1, 0, 0), "Complex vector []");

	//Find
/*	auto it = std::find(vec.begin(), vec.end(), complex_element(0, 1, 0));
	assert_test(std::distance(vec.begin(), it) == 1, "Complex vector + std::find");

	//Insertion
	vec.insert(it, complex_element(1, 1, 1));
	assert_test(vec[0] == complex_element(0, 0, 1), "Complex vector insertion");
	assert_test(vec[1] == complex_element(1, 1, 1), "Complex vector insertion");
	assert_test(vec[2] == complex_element(0, 1, 0), "Complex vector insertion");
	assert_test(vec[3] == complex_element(1, 0, 0), "Complex vector insertion");

	//Writing to iterator
	*it = complex_element(9, 9, 9);
	assert_test(vec[1] == complex_element(9, 9, 9), "Writing to iterator");

	//Assignment operator
	complex_element el;
	el = *it;
	assert_test(el == complex_element(9, 9, 9), "Assignment operator");

	//Sorting
	vec.clear();
	{
		auto gen = std::bind(uniform_int_distribution<int>(0, 999), default_random_engine());

		std::vector<int> correct_vec;

		for(int i = 0; i < 1000; ++i)
			correct_vec.push_back(gen());

		//correct_vec.push_back(612);

		for(auto val : correct_vec)
			vec.push_back(complex_element(val));

		auto tp1 = chrono::high_resolution_clock::now();
		std::sort(correct_vec.begin(), correct_vec.end());
		auto tp2 = chrono::high_resolution_clock::now();
		std::sort(vec.begin(), vec.end());
		auto tp3 = chrono::high_resolution_clock::now();

		double simple_timecost = duration_cast<microseconds>(tp2 - tp1).count() * 1e-6, 
			complex_timecost = duration_cast<microseconds>(tp3 - tp2).count() * 1e-6;

		cout << "Complex timecost is " << complex_timecost / simple_timecost << " times worse" << std::endl;

		bool r = true;
		for (int i = 0; i < vec.size(); ++i)
			r = r && (vec[i].val() == correct_vec[i]);
		
		assert_test(r, "Sorting");

		if (!r)
		{
			for (int i = 0; i < vec.size(); ++i)
				cout << correct_vec[i] << '\t' << vec[i].val() << std::endl;
		}
	}*/
}

void test_algorithm()
{
	//Unique vector
	{
		std::array<int, 8> vec = { 3, 8, 9, 39, 42, 55, 56, 59 };
		
		auto it = find_first_ge(vec.begin(), vec.end(), 2);
		assert_test(std::distance(vec.begin(), it) == 0, "find_first_ge unique middle");

		it = find_first_ge(vec.begin(), vec.end(), 3);
		assert_test(std::distance(vec.begin(), it) == 0, "find_first_ge unique middle");

		it = find_first_ge(vec.begin(), vec.end(), 4);
		assert_test(std::distance(vec.begin(), it) == 1, "find_first_ge unique middle");
	}
	//Repeated fragments
	{
		std::array<int, 8> vec = { 3, 8, 8, 39, 42, 42, 42, 59 };

		auto it_1 = find_first_ge(vec.begin(), vec.end(), 1);
		assert_test(std::distance(vec.begin(), it_1) == 0, "find_first_ge left limit");

		auto it_60 = find_first_ge(vec.begin(), vec.end(), 60);
		assert_test(std::distance(vec.begin(), it_60), "find_first_ge right limit");

		auto it_8 = find_first_ge(vec.begin(), vec.end(), 8);
		assert_test(std::distance(vec.begin(), it_8) == 1, "find_first_ge middle");

		auto it_39 = find_first_ge(vec.begin(), vec.end(), 39);
		assert_test(std::distance(vec.begin(), it_39) == 3, "find_first_ge middle");

		auto it_42 = find_first_ge(vec.begin(), vec.end(), 42);
		assert_test(std::distance(vec.begin(), it_42) == 4, "find_first_ge middle");

		auto it_59 = find_first_ge(vec.begin(), vec.end(), 59);
		assert_test(std::distance(vec.begin(), it_59) == 7, "find_first_ge middle");
	}
}

void test_complex_hashmap()
{
	test_algorithm();

	
	complex_element_streamer streamer;

	//Internal
	{
		auto gen = std::bind(uniform_int_distribution<int>(0, 999), default_random_engine());
		std::unordered_set<complex_element> correct_set;
	
		//buffered_complex_hashset<complex_element, complex_element_streamer> hset(streamer);	
		hashset_generator<complex_element, complex_element_streamer, 128U>::generate<hashset_t::Internal>::result hset(streamer);

		int count = 0;
		//Insertion test
		for (int i = 0; i < 2000; ++i)
		{
			int val = gen();
			//cout << "Inserting " << val << std::endl;

			auto r2 = correct_set.insert(val);

			if (i == 5)
			{
				++count;
				//hset.print_debug();
			}
			if (val == 553)
				int x = 0;
			
			//cout << i << std::endl;
			auto r1 = hset.insert(val);
			//hset.print_debug();

			bool crct = (r1 == r2.second);
			if (!crct)
				int x = 0;
			assert_test(crct, "Complex hashset insertion");
			assert_test(hset.size() == correct_set.size(), "Complex hashset size");
		}
		

		assert_test(hset.size() == correct_set.size(), "Complex hashset size");

		//Find test
		for (int i = 0; i < 2000; ++i)
		{
			int val = gen();
			auto it1 = correct_set.find(val);
			auto it2 = hset.find(val);

			bool r1 = (it1 == correct_set.end());
			bool r2 = (it2 == hset.end());

			assert_test(r1 == r2, "Complex hashset find");
		}
	}

	//Buffered
	{
		auto gen = std::bind(uniform_int_distribution<int>(0, 999), default_random_engine());
		std::unordered_set<complex_element> correct_set;

		//buffered_complex_hashset<complex_element, complex_element_streamer> hset(streamer);	
		hashset_generator<complex_element, complex_element_streamer, 4096U>::generate<hashset_t::Buffered>::result hset(streamer);

		int count = 0;
		//Insertion test
		for (int i = 0; i < 2000; ++i)
		{
			int val = gen();
			//cout << "Inserting " << val << std::endl;

			auto r2 = correct_set.insert(val);

			if (i == 36)
			{
				++count;
				//hset.print_debug();
			}
			if (val == 553)
				int x = 0;

			//cout << i << std::endl;
			auto r1 = hset.insert(val);
			//hset.print_debug();

			bool crct = (r1 == r2.second);
			if (!crct)
				int x = 0;
			assert_test(crct, "Complex hashset insertion");
			assert_test(hset.size() == correct_set.size(), "Complex hashset size");
		}


		assert_test(hset.size() == correct_set.size(), "Complex hashset size");

		//Find test
		for (int i = 0; i < 2000; ++i)
		{
			int val = gen();
			auto it1 = correct_set.find(val);
			auto it2 = hset.find(val);

			bool r1 = (it1 == correct_set.end());
			bool r2 = (it2 == hset.end());

			assert_test(r1 == r2, "Complex hashset find");
		}
	}


	//Tets add delayed
	{
		
		{
			auto gen = std::bind(uniform_int_distribution<int>(0, 999), default_random_engine());

			std::unordered_set<complex_element> correct_set;

			hashset_generator<complex_element, complex_element_streamer, 4096U, std::hash<complex_element>>::generate<hashset_t::Delayed>::result hset(streamer);

			int count = 0;
			//Insertion test
			for (int i = 0; i < 2000; ++i)
			{
				int val = gen();
				//cout << "Inserting " << val << std::endl;
				if (i == 321)
					int x = 3;

				auto r2 = correct_set.insert(val);
				hset.insert_delayed(val);
				if (i >= 214)
				{ 
					if (hset.find(324) == hset.end())
						int y = 0;
				}
			}

			//hset.insert_delayed(319);
			auto itt = hset.find(324);

			hset.flush_delayed_buffer();

			assert_test(hset.size() == correct_set.size(), "Complex hashset size");


	//		itt = hset.find(324);

			//Find test
			for (int i = 0; i < 2000; ++i)
			{
				int val = gen();
				auto it1 = correct_set.find(val);
				auto it2 = hset.find(val);

				bool r1 = (it1 == correct_set.end());
				bool r2 = (it2 == hset.end());

				assert_test(r1 == r2, "Complex hashset find");
				if (r1 != r2)
					cout << "Failed on " << val << std::endl;
			}
		}
	}
	
	//Test add_range
	{
/*		complex_element_streamer streamer;
		direct_complex_hashset<complex_element, complex_element_streamer, std::hash<complex_element>, true, 4096U> hset(streamer);

		std::unordered_set<complex_element> correct_set;

		vector<int> ins_data;

		for (int i = 0; i < 999; ++i)
			ins_data.push_back(i);
		//random_shuffle(ins_data.begin(), ins_data.end());

		std::hash<complex_element> hasher;

		for (int j = 1; j < 3; ++j)
		{
			for (int i = 0; i < 9; ++i)
			{
				auto begin_it = ins_data.begin() + i * 100, end_it = ins_data.begin() + i * 100 + 90;

				hset.insert_range(begin_it, end_it, [=](const complex_element & el){
					return hasher(el);
				}, [](const complex_element & el){
					return el;
				}, [](const complex_element & el){});

				correct_set.insert(begin_it, end_it);
			}
		}
		
		assert_test(hset.size() == correct_set.size(), "Complex hashset insert range");
		*/
	}
}

void test_complex_queue()
{
	complex_element_streamer streamer;
	complex_queue<complex_element, complex_element_streamer> cq(streamer, "test_queue.dat");

	std::queue<complex_element> correct_queue;

	auto gen = std::bind(uniform_int_distribution<int>(0, 999), default_random_engine());

	//Push
	for (int i = 0; i < 20000; ++i)
	{
		int val = gen();
		correct_queue.push(val);
		cq.push(val);
	}

	//Pop and compare
	for (int i = 0; i < 20000; ++i)
	{
		assert_test(correct_queue.front() == cq.top(), "Complex queue simple front + pop");
		correct_queue.pop();
		cq.pop();
	}

	for (int j = 0; j < 100; ++j)
	{
		//Push
		for (int i = 0; i < 1000; ++i)
		{
			int val = gen();
			correct_queue.push(val);
			cq.push(val);
		}

		//Pop and compare
		for (int i = 0; i < 1000; ++i)
		{
			assert_test(correct_queue.front() == cq.top(), "Complex queue front + pop");
			correct_queue.pop();
			cq.pop();
		}
	}
}

void test_complex_stack()
{
	complex_element_streamer streamer;
	complex_stack<complex_element, complex_element_streamer> cq(streamer);

	std::stack<complex_element> correct_stack;

	auto gen = std::bind(uniform_int_distribution<int>(0, 999), default_random_engine());

	//Push
	for (int i = 0; i < 2000; ++i)
	{
		int val = gen();
		correct_stack.push(val);
		cq.push(val);
	}

	//Pop and compare
	for (int i = 0; i < 2000; ++i)
	{
		assert_test(correct_stack.top() == cq.top(), "Complex queue front + pop");
		correct_stack.pop();
		cq.pop();
	}
}
