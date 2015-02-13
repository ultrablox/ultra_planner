
#ifndef UltraCore_sort_h
#define UltraCore_sort_h

#include <type_traits>
#include <algorithm>
#include <vector>

namespace UltraCore
{
	template<class T>
	class sorter
	{
	public:
		typedef typename T::value_type Data;
		typedef typename T::iterator Iter;

#if _DEBUG
		T * m_pArr;
#endif
	};

	/*
	Insertion sort. Has O(n^2) complexity.
	*/
	template<typename T, typename C>
	class insertion_sorter
	{
	public:
		typedef T Iter;
		typedef C Comparator;

		void operator()(Iter first, Iter last, Comparator cmp)
		{
			//Start from the second element
			Iter cur = first + 1;

			//For each elements except first
			for(; cur != last; ++cur)
			{
				//Key is in *cur
				//Iter insertion_it = first;
				//for(; cmp(*insertion_it, *cur) && (insertion_it != cur); ++insertion_it){}
				//Iter insertion_it = std::find_if_not(first, cur, std::bind2nd(cmp, *cur));
				Iter insertion_it = std::lower_bound(first, cur, *cur, cmp);

				//If insertion_it is in the end - last element is sorted
				if(insertion_it != cur)
				{
					//Move key to temp
					typename Iter::value_type tmp(std::move(*cur));

					//Move all elements on the right of insertion_it 1 position righter
					//Move key to its position
					
					for(Iter move_el = insertion_it; move_el != cur; ++move_el)
					{
						std::swap(tmp, *move_el);
					}

					*cur = std::move(tmp);
				}
			}
		}
	};

	/*
	Merge sort. Has O(N * lg(N)) complexity.
	*/
	template<typename T, typename C>
	class MergeSorter
	{
	public:
		typedef T Iter;
		typedef C Comparator;

		void operator()(Iter first, Iter last, Comparator cmp)
		{
			sort(first, last, cmp);
		}

	private:
		void sort(Iter first, Iter last, Comparator cmp)
		{
			int range_size = std::distance(first, last);
			if(range_size == 1)
				return;

			int half_size = range_size >> 2;

			Iter middle = first + half_size + 1;
			sort(first, middle, cmp);
			sort(middle, last, cmp);
			merge(first, middle, last, cmp);
		}

		void merge(Iter first, Iter middle, Iter last, Comparator cmp)
		{
			int range_size = std::distance(first, last);
			std::vector<typename Iter::value_type> cache;
			cache.reserve(range_size);

			Iter left_it = first, right_it = middle;
			while((left_it != middle) && (right_it != last))
			{
				if(cmp(*left_it, *right_it))
					cache.push_back(*left_it++);
				else
					cache.push_back(*right_it++);
			}

			while(left_it != middle)
				cache.push_back(*left_it++);

			while(right_it != last)
				cache.push_back(*right_it++);

			auto src_it = cache.begin();
			
			for(auto dst_it = first; dst_it != last; ++dst_it, ++src_it)
				*dst_it = *src_it;
		}
	};

	/*
	Heap sorter. Complexity N*lg(N)
	*/
	template<typename T, typename C>
	class HeapSorter : public sorter<T>
	{
	public:
		typedef typename T::iterator Iter;
		typedef C Comparator;

		void operator()(Iter first, Iter last, Comparator cmp)
		{
			m_first = first;
//			m_last = last;
			m_cmp = cmp;

			buildMaxHeap(last);
			
			for(Iter it = last - 1; it != first; --it)
			{
				std::swap(*first, *it);
				maxHeapify(first, it);
			}
		}

	private:
		Iter parent(Iter it)
		{
			int i = std::distance(m_first, it);
			return m_first + (i/2);
		}

		Iter left(Iter it, Iter last)
		{
			int res = 2 * std::distance(m_first, it) + 1;

			if(res < std::distance(m_first, last))
				return m_first + res;
			else
				return last;
		}

		Iter right(Iter it, Iter last)
		{
			int res = 2 * std::distance(m_first, it) + 2;

			if(res < std::distance(m_first, last))
				return m_first + res;
			else
				return last;
		}

		void buildMaxHeap(Iter last)
		{
			for(auto it = m_first + (std::distance(m_first, last) / 2 - 1); ; --it)
			{
				maxHeapify(it, last);
				if(it == m_first)
					break;
			}
		}

		void maxHeapify(Iter it, Iter last)
		{
			Iter lit = left(it, last), rit = right(it, last), largest;

			if((lit != last) && !m_cmp(*lit, *it))
				largest = lit;
			else
				largest = it;

			if((rit != last) && !m_cmp(*rit, *largest))
				largest = rit;

			if(largest != it)
			{
				std::swap(*largest, *it);
				maxHeapify(largest, last);
			}
		}

	private:
		Iter m_first;//, m_last;
		Comparator m_cmp;
		
	};

	/*
	Counting sort
	*/
	template<typename T>
	class CountSorter : public sorter<T>
	{
		using Data = typename sorter<T>::Data;
		using Iter = typename sorter<T>::Iter;
	public:
		
		static_assert(std::is_integral<Data>::value, "Element type should be integral type.");

		void operator()(Iter first, Iter last)
		{
			//Find maximum
			Data max = *(std::max_element(first, last, std::less<typename T::value_type>()));

			sort(first, last, max);
		}

		void sort(Iter first, Iter last, Data max_value)
		{
			std::vector<int> tmp;
			const int tmp_size = max_value + 1;
			tmp.resize(tmp_size);

			//Initialize to 0
			memset(tmp.data(), 0, sizeof(int) * tmp_size);

			//Fill tmp with element count
			for(auto it = first; it != last; ++it)
				++tmp[*it];

			//Now tmp contains number of each elements

			for(int i = 1; i < tmp_size; ++i)
				tmp[i] += tmp[i-1];

			T res;
			res.resize(std::distance(first, last));

			for(Iter it = first; it != last; ++it)
			{
				res[tmp[*it]-1] = *it;
				--tmp[*it];
			}

			for(Iter dit = first, cit = res.begin(); dit != last; ++dit, ++cit)
				*dit = *cit;
		}
	};

	/*
	Counting sort
	*/
	template<typename T>
	class RadixSorter : public sorter<T>
	{
		using Data = typename sorter<T>::Data;
		using Iter = typename sorter<T>::Iter;
	public:
		
		static_assert(std::is_integral<Data>::value, "Element type should be integral type.");

		void operator()(Iter first, Iter last)
		{
			
		}
	};


	template<template<typename T, typename A> class S, typename A, typename C>
	void ultra_sort(A & container, C cmp)
	{
		S<A, C> sorter;
#if _DEBUG
		sorter.m_pArr = &container;
#endif
		sorter(container.begin(), container.end(), cmp);
	}

	template<template<typename T> class S, typename A>
	void ultra_sort(A & container)
	{
		S<A> sorter;
#if _DEBUG
		sorter.m_pArr = &container;
#endif
		sorter(container.begin(), container.end());
	}

	template<typename T, typename C>
	void insertion_sort(T first, T last, C cmp)
	{
		ultra_sort<insertion_sorter>(first, last, cmp);
	}
};

#endif
