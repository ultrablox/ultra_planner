
#ifndef UltraCore_complex_hashset_generator_h
#define UltraCore_complex_hashset_generator_h

#include "buffered_complex_hashset.h"
#include "direct_complex_hashset.h"
#include "delayed_complex_hashset.h"
#include "block.h"

enum class hashset_t {Internal, Buffered, Delayed};

template<typename T, typename S, typename H>
class hashset_generator
{
	using block_t = hashset_block<4096U>;	//65536U //4096U // 8192U

public:
	template<hashset_t Tp>
	struct generate
	{
	};

	template<>
	struct generate<hashset_t::Internal>
	{
	};

	template<>
	struct generate<hashset_t::Buffered>
	{
		using storage_wrapper_t = vector_storage_wrapper<cached_file<block_t, 200000>>;//65536U
		using result = buffered_complex_hashset<T, S, storage_wrapper_t, H>;
	};

	template<>
	struct generate<hashset_t::Delayed>
	{
		using storage_t = data_file<block_t>;
		using result = delayed_complex_hashset<T, S, storage_t, H>;
	};
};

#endif
