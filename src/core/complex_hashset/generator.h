
#ifndef UltraCore_complex_hashset_generator_h
#define UltraCore_complex_hashset_generator_h

#include "buffered_complex_hashset.h"
#include "direct_complex_hashset.h"
#include "delayed_complex_hashset.h"
#include "block.h"

enum class hashset_t {Internal, Buffered, Delayed};

template<typename T, typename S, unsigned int BlockSize = 4096U, typename H = std::hash<T>>//65536U //4096U // 8192U, unsigned int BlockSize = 4096U
class hashset_generator
{
	using block_t = hashset_block<BlockSize>;

public:
	template<hashset_t Tp, bool = true>	//By default it uses hashset_t::Internal
	struct generate
	{
		/*using storage_wrapper_t = vector_wrapper<std::vector<block_t>>;
		using result = buffered_complex_hashset<T, S, storage_wrapper_t, H>;*/
	};

	template<bool dummy>
	struct generate<hashset_t::Internal, dummy>
	{
		using storage_wrapper_t = vector_wrapper<std::vector<block_t>>;
		using result = buffered_complex_hashset<T, S, storage_wrapper_t, H>;
	};

	template<bool dummy>
	struct generate<hashset_t::Buffered, dummy>
	{
		using storage_wrapper_t = cached_file_wrapper<cached_file<block_t, 200000>>;//65536U //500000
		using result = buffered_complex_hashset<T, S, storage_wrapper_t, H>;
	};

	template<bool dummy>
	struct generate<hashset_t::Delayed, dummy>
	{
		using storage_t = data_file<block_t>;
		using result = delayed_complex_hashset<T, S, storage_t, H>;
	};/**/
};

#endif
