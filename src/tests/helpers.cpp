
#include "helpers.h"
#include <iostream>

using namespace std;

void assert_test(bool val, const std::string & description)
{
	if(!val)
		cout << "Test '" << description << "' failed\n";
}
