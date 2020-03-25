#include "tests.h"
#include "../include/muu/aligned_alloc.h"

namespace
{
	[[nodiscard]]
	static uintptr_t pointer_alignment(const void* ptr) noexcept
	{
		if (!ptr)
			return 0_sz;
		return bit_floor(pointer_cast<uintptr_t>(ptr));
	}
}

TEST_CASE("aligned_alloc")
{
	static constexpr size_t test_size_max = 1024_sz * 1024_sz * 16_sz; // 16 mb

	std::vector<int32_t> vals;
	vals.reserve(test_size_max / sizeof(int32_t));
	for (size_t i = 0; i < test_size_max / sizeof(int32_t); i++)
		vals.push_back(rand());

	for (size_t align = 1_sz; align <= impl::aligned_alloc_max_alignment; align *= 2_sz)
	{
		// check that zero alignments fail
		{
			auto ptr = muu::aligned_alloc(0_sz, align);
			REQUIRE(ptr == nullptr);
		}

		// check that zero sizes fail
		{
			auto ptr = muu::aligned_alloc(align, 0_sz);
			REQUIRE(ptr == nullptr);
		}

		for (size_t size = 256_sz; size <= test_size_max; size *= 2_sz)
		{
			INFO("Testing aligned_alloc with align = "sv << align << " and size = "sv << size)

			void* ptr{};

			// check that non-pow2 alignments fail
			if (align > 1_sz)
			{
				ptr = muu::aligned_alloc(align + 1_sz, size);
				REQUIRE(ptr == nullptr);
			}

			// check that trying to over-align past the limit fails
			if (align == impl::aligned_alloc_max_alignment)
			{
				ptr = muu::aligned_alloc(impl::aligned_alloc_max_alignment << 1, size);
				REQUIRE(ptr == nullptr);
			}

			// check that basic, correct aligned_alloc() calls work
			ptr = muu::aligned_alloc(align, size);
			REQUIRE(ptr != nullptr);
			CHECK(pointer_alignment(ptr) >= align);

			// check writing to the whole range
			memcpy(ptr, vals.data(), size);
			CHECK(memcmp(ptr, vals.data(), size) == 0);

			// check shrinking still large enough to just re-use the same buffer
			{
				auto ptr2 = aligned_realloc(ptr, size * 3_sz / 4_sz);
				REQUIRE(ptr2 == ptr);
			}

			// check growth
			ptr = aligned_realloc(ptr, size * 2_sz);
			REQUIRE(ptr != nullptr);
			CHECK(pointer_alignment(ptr) >= align);
			CHECK(memcmp(ptr, vals.data(), size) == 0);

			// check shrinking
			ptr = aligned_realloc(ptr, size / 4_sz);
			REQUIRE(ptr != nullptr);
			CHECK(pointer_alignment(ptr) >= align);
			CHECK(memcmp(ptr, vals.data(), size / 4_sz) == 0);

			// check that trying to realloc with zero size fails
			{
				auto ptr2 = aligned_realloc(ptr, 0_sz);
				REQUIRE(ptr2 == nullptr);
			}

			// check that trying to realloc with a nullptr creates a new alloc with default alignment
			{
				auto ptr2 = aligned_realloc(nullptr, size);
				REQUIRE(ptr2 != nullptr);
				REQUIRE(ptr2 != ptr);
				CHECK(pointer_alignment(ptr2) >= __STDCPP_DEFAULT_NEW_ALIGNMENT__);
				aligned_free(ptr2);
			}

			aligned_free(ptr);
		}
	}
}
