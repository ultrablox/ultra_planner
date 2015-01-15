#ifndef UltraCore_algorithm_h
#define UltraCore_algorithm_h

/*
Returns iterator to the first element greater or equal to
given from given sorted sequence.
*/

template<typename It, typename Val>
It find_first_ge(It begin, It end, const Val & val)
{
	if (begin == end)
		return end;

	//Check limits first
	if (val <= *begin)
		return begin;
	else if (val > *(end - 1))
		return end;
	else
	{
		//Else use binary search
		size_t count = end - begin, step;
		while (count > 1)
		{
			step = (count / 2);
			//It middle_it = begin + step;

			//Check that edge crosses equal elements
			/*if (*(middle_it - 1) < val)
				begin = middle_it;
			else
				end = middle_it;*/
			if (*(begin + step - 1) < val)
				begin += step;
			else
				end = begin + step;

			count = end - begin;
		}
		

		return begin;
	}
}
#endif
