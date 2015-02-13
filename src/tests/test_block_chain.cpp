
#include <core/complex_hashset/block_chain.h>
#include "test_helpers.h"

void test_block_chain()
{

	//Operator++
	chain_address addr(0, 0, 129);
	int i = 0;
	for (; i < 1000; ++i, ++addr)
		assert_test(addr.linear_address() == i, "++chain_address");

	//Operator--
	for (; i > 0; --i, --addr)
		assert_test(addr.linear_address() == i, "++chain_address");

	//Operator+=
	for (; i < 1000; i += 71, addr += 71)
		assert_test(addr.linear_address() == i, "++chain_address");
}