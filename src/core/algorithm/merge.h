
#ifndef UltraCore_merge_h
#define UltraCore_merge_h

#include <functional>
#include <iostream>

namespace UltraCore
{

	/*
	Merges to linear containers with integer keys. Both 
	containers must be sorted by their integer keys. Result is placed
	into first container.
	*/
	
	
	/*
	Takes as input a linear sorted container of elements with integer keys. Checks
	if elements with same keys are the same and overrides duplications. Doesn't alter
	container size. Returns iterator following the last unique element.
	*/
	template<typename Iter, typename KeyExtractor, typename Equality = std::equal_to<typename Iter::value_type>>
	Iter unique(Iter first, Iter last, KeyExtractor keyer, Equality eq = Equality())
	{
		auto result = first;

		while(first != last)
		{
			//Find group end with the same key
			auto end_it = first;
			while((end_it != last) && (keyer(*end_it) == keyer(*first)))
				++end_it;

			//If group consists from 1 element - element is unique
			int group_elem_count = std::distance(first, end_it);
			if(group_elem_count != 1)
			{
				auto last_unique_it = std::unique(first, end_it, eq);

				int unique_count = std::distance(first, last_unique_it);

				//Move unique elements
				while(first != last_unique_it)
					*(result++) = *(first++);

				//Skip bad elements by first iterator
				std::advance(first, group_elem_count - unique_count);
			}
			else
				*(result++) = *(first++);
		}

		return result;
	}

	/*
	Takes as input two linear sorted containers with unique elements and
	integer keys. Removes all elements from second container which are presented
	in second container.
	*/
	template<typename It, typename Iter, typename ExtKey, typename MainKey, typename Equality>
	Iter unique(It ext_begin, It ext_end, Iter first, Iter last, ExtKey extKeyGetter, MainKey mainKeyGetter, Equality eq)
	{
		//auto first = main_cont.begin(), last = main_cont.end();
		auto result = first;

		auto ext_it = ext_begin;

		while (ext_it != ext_end)
		{
			//Move main iterator
			while((first != last) && (mainKeyGetter(*first) < extKeyGetter(*ext_it)))
				*(result++) = *(first++);

			//Check we havent reached end
			if(first == last)
				break;

			//Move external iterator
			while ((ext_it != ext_end) && (extKeyGetter(*ext_it) < mainKeyGetter(*first)))
				++ext_it;

			//Check we havent reached end
			if (ext_it == ext_end)
				break;

			//If keys are not equal - iterate again
			if(mainKeyGetter(*first) != extKeyGetter(*ext_it))
				continue;

			auto key = extKeyGetter(*ext_it);

			//Determine external group with same keys
			auto ext_group_end_it = ext_it;
			//while (key == extKeyGetter(*ext_group_end_it))
			while ((ext_group_end_it != ext_end) && (key == extKeyGetter(*ext_group_end_it)))
				++ext_group_end_it;	
				

			//Iterate each element in main container with same key
			bool is_duplication;
			while((first != last) && (mainKeyGetter(*first) == key))
			{
				auto ext_eq_it = std::find_if(ext_it, ext_group_end_it, [=](const typename It::value_type & val){
					return eq(val, *first);
				});

				//If element is duplication - skip it
				if(ext_eq_it != ext_group_end_it)
					++first;
				else
					*(result++) = *(first++);
			}

			//Move external iterator to the end of the group
			ext_it = ext_group_end_it;
		}

		//Shrink tail with keys greater than external values
		while(first != last)
			*(result++) = *(first++);
		return result;
	}

	/*
	Analogue of std::merge, uses only assignment operator.
	*/
	template<typename It1, typename It2, typename OutIt, typename KeyLess>
	void merge(It1 first1, It1 last1, It2 first2, It2 last2, OutIt result, KeyLess cmp)
	{
		size_t s2 = std::distance(first2, last2);

		while (true)
		{
			if (first1 == last1)
			{
				for (auto it = first2; it != last2; ++it)
					*result++ = *it;
				break;
			}

			if (first2 == last2)
			{
				for (auto it = first1; it != last1; ++it)
					*result++ = *it;
				break;
			}
			
			*result++ = cmp(*first2, *first1) ? *first2++ : *first1++;
		}
		/*auto result = rDestBegin;
		auto dest_it = rDestDataBegin;
		auto src_it = rSrcBegin;

		while (result != rDestEnd)
		{
			if(src_it == rSrcEnd) //Source is inserted - destination is on its place
				break;

			if(dest_it == rDestEnd) //We are in the end - just stop
				break;

			cout << "D:" << *dest_it << ", S:" << *src_it << ", src_dist=" << std::distance(src_it, rSrcEnd) << ", dst_dist=" << std::distance(dest_it, rDestEnd) << "\n";
			if(cmp(*dest_it, *src_it))
			{
				*result++ = std::move(*src_it++);
			}
			else
			{
				*result++ = std::move(*dest_it++);
			}

			//cout <<  << "\n";
		}*/
	}
	
	/*
	Returns iterator to the ith element: e[i] != e[i-1], where i is the most value
	i <= max_size. Or it can return the end of first equal group, if its size is
	more than max_size.
	*/
	template<typename It, typename Pred>
	It find_group_end(It begin, It end, int max_size, Pred pred)
	{
		It last_change = begin;
		
		for (It prev_it = begin, it = begin + 1; (it != end) && (max_size > 0); ++it, ++prev_it, --max_size)
		{
			if (pred(*prev_it) != pred(*it))	//We found change
				last_change = it;
		}

		if (last_change == begin)
		{
			last_change = std::find_if(begin, end, [=](const typename It::value_type & val){
				return pred(*begin) != pred(val);
			});
		}
		
		return last_change;
	}

};

#endif