
#include "test_helpers.h"
#include <core/containers/external_queue.h>
#include <queue>
#include <random>

struct test_block
{
	int val;
	char data[4096 - sizeof(int)];

	friend bool operator==(const test_block & lhs, const test_block & rhs)
	{
		return lhs.val == rhs.val;
	}
};

void test_external_queue_complex()
{
	external_queue<test_block> queue("test_queue");
	std::queue<test_block> correct_queue;

	auto seq_gen = std::bind(uniform_int_distribution<int>(25, 153), default_random_engine());
	auto val_gen = std::bind(uniform_int_distribution<int>(0, 42153), default_random_engine());

	for (int i = 0; i < 19; ++i)
	{
		int seq_size = seq_gen();
		
		for (int j = 0; j < seq_size; ++j)
		{
			test_block el;
			el.val = val_gen();

			queue.push(el);
			correct_queue.push(el);
		}

		int read_size = seq_size % 11;

		for (int j = 0; j < read_size; ++j)
		{
			assert_test(queue.front() == correct_queue.front(), "External Queue complex");

			queue.pop();
			correct_queue.pop();
		}
	}


	while (!correct_queue.empty())
	{
		assert_test(queue.front() == correct_queue.front(), "External Queue complex");

		queue.pop();
		correct_queue.pop();
	}

	assert_test(correct_queue.empty() == queue.empty(), "External Queue complex");
}

void test_external_queue_base()
{
	external_queue<test_block> queue("test_queue");
	std::queue<test_block> correct_queue;

	
	auto val_gen = std::bind(uniform_int_distribution<int>(0, 15312), default_random_engine());

	for (int i = 0; i < 10000; ++i)
	{
		test_block el;
		el.val = val_gen();

		queue.push(el);
		correct_queue.push(el);
	}

	while (!correct_queue.empty())
	{
		assert_test(queue.front() == correct_queue.front(), "External Queue base");

		queue.pop();
		correct_queue.pop();
	}

	assert_test(correct_queue.empty() == queue.empty(), "External Queue base empty");
}

void test_external_containers()
{
	test_external_queue_base();
	test_external_queue_complex();
}