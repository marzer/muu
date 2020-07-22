#include "tests.h"
#include "../include/muu/tagged_ptr.h"
#include <array>

MUU_PUSH_WARNINGS
MUU_DISABLE_INIT_WARNINGS
MUU_DISABLE_PADDING_WARNINGS

// check all static invariants
template <typename T, size_t min_align>
struct tagged_ptr_static_checks final
{
	// type traits
	using tptr = tagged_ptr<T, min_align>;
	static_assert(sizeof(tptr) == sizeof(T*));
	static_assert(std::is_trivially_copyable_v<tptr>);
	static_assert(std::is_trivially_destructible_v<tptr>);

	// static members and typedefs
	static_assert(std::is_same_v<typename tptr::element_type, T>);
	static_assert(std::is_same_v<typename tptr::pointer, T*>);
	static_assert(std::is_same_v<typename tptr::const_pointer, const T*>);
	static_assert(tptr::minimum_alignment == min_align);
	static_assert(tptr::tag_bit_count >= bit_width(min_align - 1_sz));

	// tag size
	using tag_type = typename tptr::tag_type;
	static_assert(sizeof(tag_type) * build::bits_per_byte >= tptr::tag_bit_count);
	static_assert(tptr::max_tag == bit_fill_right<tag_type>(tptr::tag_bit_count));
};
#if MUU_ARCH_AMD64
template struct tagged_ptr_static_checks<void, 1    >;
#endif
template struct tagged_ptr_static_checks<void, 2    >;
template struct tagged_ptr_static_checks<void, 4    >;
template struct tagged_ptr_static_checks<void, 8    >;
template struct tagged_ptr_static_checks<void, 16   >;
template struct tagged_ptr_static_checks<void, 32   >;
template struct tagged_ptr_static_checks<void, 64   >;
template struct tagged_ptr_static_checks<void, 128  >;
template struct tagged_ptr_static_checks<void, 256  >;
template struct tagged_ptr_static_checks<void, 512  >;
template struct tagged_ptr_static_checks<void, 1024 >;
template struct tagged_ptr_static_checks<void, 2048 >;
template struct tagged_ptr_static_checks<void, 4096 >;
template struct tagged_ptr_static_checks<void, 8192 >;
template struct tagged_ptr_static_checks<void, 16384>;
template struct tagged_ptr_static_checks<void, 32768>;
template struct tagged_ptr_static_checks<int32_t, 4    >;
template struct tagged_ptr_static_checks<int32_t, 8    >;
template struct tagged_ptr_static_checks<int32_t, 16   >;
template struct tagged_ptr_static_checks<int32_t, 32   >;
template struct tagged_ptr_static_checks<int32_t, 64   >;
template struct tagged_ptr_static_checks<int32_t, 128  >;
template struct tagged_ptr_static_checks<int32_t, 256  >;
template struct tagged_ptr_static_checks<int32_t, 512  >;
template struct tagged_ptr_static_checks<int32_t, 1024 >;
template struct tagged_ptr_static_checks<int32_t, 2048 >;
template struct tagged_ptr_static_checks<int32_t, 4096 >;
template struct tagged_ptr_static_checks<int32_t, 8192 >;
template struct tagged_ptr_static_checks<int32_t, 16384>;
template struct tagged_ptr_static_checks<int32_t, 32768>;
template struct tagged_ptr_static_checks<int64_t, 8    >;
template struct tagged_ptr_static_checks<int64_t, 16   >;
template struct tagged_ptr_static_checks<int64_t, 32   >;
template struct tagged_ptr_static_checks<int64_t, 64   >;
template struct tagged_ptr_static_checks<int64_t, 128  >;
template struct tagged_ptr_static_checks<int64_t, 256  >;
template struct tagged_ptr_static_checks<int64_t, 512  >;
template struct tagged_ptr_static_checks<int64_t, 1024 >;
template struct tagged_ptr_static_checks<int64_t, 2048 >;
template struct tagged_ptr_static_checks<int64_t, 4096 >;
template struct tagged_ptr_static_checks<int64_t, 8192 >;
template struct tagged_ptr_static_checks<int64_t, 16384>;
template struct tagged_ptr_static_checks<int64_t, 32768>;

// check that noexcept propagation works correctly for function pointers
static_assert(!noexcept(std::declval<tagged_ptr<int(), 4>>()()));
static_assert(noexcept(std::declval<tagged_ptr<int()noexcept, 4>>()()));

// check deduction guides
static_assert(std::is_same_v<decltype(tagged_ptr{ std::add_pointer_t<int>{}, 0u }), tagged_ptr<int>>);
static_assert(std::is_same_v<decltype(tagged_ptr{ std::add_pointer_t<int>{} }), tagged_ptr<int>>);

namespace
{
	template <size_t align>
	struct aligned
	{
		alignas(align) uint8_t kek;
	};
	static_assert(alignof(aligned<32>) == 32);
}

TEST_CASE("tagged_ptr - basic initialization")
{
	using tp = tagged_ptr<void, 16>; //4 free bits
	static_assert(sizeof(tp) == sizeof(void*));

	tp val;
	CHECK(val.ptr() == pointer_cast<void*>(nullptr));
	CHECK(val == pointer_cast<void*>(nullptr));
	CHECK(val == pointer_cast<const void*>(nullptr));
	CHECK(pointer_cast<void*>(nullptr) == val);
	CHECK(pointer_cast<const void*>(nullptr) == val);
	CHECK(val.tag() == 0u);

	auto ptr = pointer_cast<void*>(uintptr_t{ 0x12345670u });
	val = tp{ ptr };
	CHECK(val.ptr() == ptr);
	CHECK(val.tag() == 0u);

	val = tp{ ptr, 0b1010u };
	CHECK(val.ptr() == ptr);
	CHECK(val.tag() == 0b1010u);

	if constexpr (MUU_ARCH_AMD64)
	{
		//see https://en.wikipedia.org/wiki/X86-64#Virtual_address_space_details
		ptr = pointer_cast<void*>(static_cast<uintptr_t>(0xFFFF800000000000ull));
		val = tp{ ptr };
		CHECK(val.ptr() == ptr);
		CHECK(val.tag() == 0u);

		val = tp{ ptr, 0b1010u };
		CHECK(val.ptr() == ptr);
		CHECK(val.tag() == 0b1010u);
	}
}

TEST_CASE("tagged_ptr - integral tags")
{
	using tp = tagged_ptr<void, 16>; //4 free bits
	static_assert(sizeof(tp) == sizeof(void*));

	auto ptr = pointer_cast<void*>(uintptr_t{ 0x12345670u });
	tp val{ ptr, 0b1100u };
	CHECK(val.ptr() == ptr);
	CHECK(val == ptr);
	CHECK(val.tag() == 0b1100u);

	val.tag(0b1111u);
	CHECK(val.tag() == 0b1111u);
	val.tag(0b0000u);
	CHECK(val.tag() == 0b0000u);
	val.tag(0b11111111u);
	if constexpr (impl::tptr_addr_free_bits >= 4)
	{
		CHECK(val.tag() == 0b11111111u);
	}
	else if constexpr (!impl::tptr_addr_free_bits)
	{
		CHECK(val.tag() != 0b11111111u);
		CHECK(val.tag() == 0b1111u);
	}
}

// gcc bug spam, see: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=92519
#if MUU_GCC <= 9
	#define GCC_INITIALIZER_SPAM_FIX	= {}
#else
	#define GCC_INITIALIZER_SPAM_FIX
#endif

TEST_CASE("tagged_ptr - pod tags")
{
	struct data
	{
		char val GCC_INITIALIZER_SPAM_FIX;
	};
	static_assert(sizeof(data) == 1);

	using align256 = aligned<256>; //8 free bits
	static_assert(alignof(align256) == 256);

	tagged_ptr<align256> ptr;
	static_assert(sizeof(ptr) == sizeof(void*));
	static_assert(decltype(ptr)::tag_bit_count >= sizeof(data) * build::bits_per_byte);

	CHECK(ptr.ptr() == nullptr);
	CHECK(ptr == nullptr);
	CHECK(ptr.tag() == 0u);
	ptr.tag(data{ 'k' });
	CHECK(ptr.ptr() == nullptr);
	CHECK(ptr.tag() != 0u);
	CHECK(ptr.tag<data>().val == 'k');

	std::unique_ptr<align256> aligned{ new align256 };
	ptr = aligned.get();
	CHECK(ptr.ptr() == aligned.get());
	CHECK(ptr.tag<data>().val == 'k');

	ptr = tagged_ptr<align256>{};
	CHECK(ptr.ptr() == nullptr);
	CHECK(ptr.tag() == 0u);

	ptr = { aligned.get(), data{ 'k' } };
	CHECK(ptr.ptr() == aligned.get());
	CHECK(ptr.tag<data>().val == 'k');
}

#if MUU_ARCH_AMD64

TEST_CASE("tagged_ptr - pod tags (large)")
{
	struct data
	{
		char val[3] GCC_INITIALIZER_SPAM_FIX;
	};
	static_assert(sizeof(data) == 3);

	using aligned8192 = aligned<8192>;
	static_assert(alignof(aligned8192) == 8192);

	tagged_ptr<aligned8192> ptr;
	static_assert(sizeof(ptr) == sizeof(void*));
	static_assert(decltype(ptr)::tag_bit_count >= sizeof(data) * build::bits_per_byte);

	CHECK(ptr.ptr() == nullptr);
	CHECK(ptr == nullptr);
	CHECK(ptr.tag() == 0u);
	ptr.tag(data{ {'a', 'k', 'z'} });
	CHECK(ptr.ptr() == nullptr);
	CHECK(ptr.tag() != 0u);
	CHECK(ptr.tag<data>().val[0] == 'a');
	CHECK(ptr.tag<data>().val[1] == 'k');
	CHECK(ptr.tag<data>().val[2] == 'z');

	std::unique_ptr<aligned8192> aligned{ new aligned8192 };
	ptr = aligned.get();
	CHECK(ptr.ptr() == aligned.get());
	CHECK(ptr.tag<data>().val[0] == 'a');
	CHECK(ptr.tag<data>().val[1] == 'k');
	CHECK(ptr.tag<data>().val[2] == 'z');

	ptr = tagged_ptr<aligned8192>{};
	CHECK(ptr.ptr() == nullptr);
	CHECK(ptr.tag() == 0u);

	ptr = { aligned.get(), data{ {'a', 'k', 'z'} } };
	CHECK(ptr.ptr() == aligned.get());
	CHECK(ptr.tag<data>().val[0] == 'a');
	CHECK(ptr.tag<data>().val[1] == 'k');
	CHECK(ptr.tag<data>().val[2] == 'z');
}

#endif

TEST_CASE("tagged_ptr - alignments")
{
	using align32 = aligned<32>; //5 free bits

	tagged_ptr<align32> ptr;
	static_assert(sizeof(ptr) == sizeof(void*));
	using tag_type = decltype(ptr)::tag_type;
	constexpr auto filled_tag = bit_fill_right<tag_type>(sizeof(tag_type) * CHAR_BIT);
	ptr.tag(filled_tag);
	const auto expectedTag = ptr.tag();
	CHECK(expectedTag != filled_tag);
	CHECK(expectedTag == tagged_ptr<align32>::max_tag);

	std::unique_ptr<align32> aligned{ new align32 };
	ptr = aligned.get();
	CHECK(ptr.ptr() == aligned.get());
	CHECK(ptr.tag() == expectedTag);

	auto unaligned = apply_offset(aligned.get(), 1);
	CHECK(pointer_cast<std::byte*>(aligned.get()) != pointer_cast<std::byte*>(unaligned) + 1);
	ptr = unaligned; //low bit of the pointer should get masked off
	CHECK(ptr.ptr() != unaligned);
	CHECK(ptr.ptr() == aligned.get());
	CHECK(ptr.tag() == expectedTag);
}

TEST_CASE("tagged_ptr - operators")
{
	struct vec3i
	{
		int32_t x, y, z;
	};
	std::array<vec3i, 10> vecs;
	tagged_ptr<vec3i> ptr{ &vecs[0], tagged_ptr<vec3i>::max_tag };
	tagged_ptr<vec3i> ptr2{ &vecs[5], tagged_ptr<vec3i>::max_tag };
	static_assert(sizeof(ptr) == sizeof(void*));

	CHECK(ptr);
	CHECK(!!ptr);
	CHECK(&vecs[0] == ptr);
	CHECK(&vecs[5] == ptr2);

	ptr2 = &vecs[8];
	CHECK(&vecs[8] == ptr2);
	CHECK(ptr2.ptr() - &vecs[0] == 8);
	CHECK(&vecs[0] - ptr2.ptr() == -8);
	ptr2 = ptr2.ptr() + 1;
	CHECK(&vecs[9] == ptr2);
	CHECK(ptr2.ptr() - &vecs[0] == 9);
	CHECK(&vecs[0] - ptr2.ptr() == -9);
	ptr2 = ptr2.ptr() - 3;
	CHECK(&vecs[6] == ptr2);
	CHECK(ptr2.ptr() - &vecs[0] == 6);
	CHECK(&vecs[0] - ptr2.ptr() == -6);
	ptr2 = ptr2.ptr() - 1;
	CHECK(&vecs[5] == ptr2);
	CHECK(ptr2.ptr() - &vecs[0] == 5);
	CHECK(&vecs[0] - ptr2.ptr() == -5);
	ptr2 = ptr2.ptr() + 1;
	CHECK(&vecs[6] == ptr2);
	CHECK(ptr2.ptr() - &vecs[0] == 6);
	CHECK(&vecs[0] - ptr2.ptr() == -6);

	CHECK(&vecs[0] == &(*ptr));
	CHECK(&vecs[6] == &(*ptr2));

	vecs[0] = { 1, 2, 3 };
	CHECK((*ptr).x == 1);
	CHECK((*ptr).y == 2);
	CHECK((*ptr).z == 3);
	CHECK(ptr->x == 1);
	CHECK(ptr->y == 2);
	CHECK(ptr->z == 3);
	vecs[6] = { 4, 5, 6 };
	CHECK((*ptr2).x == 4);
	CHECK((*ptr2).y == 5);
	CHECK((*ptr2).z == 6);
	CHECK(ptr2->x == 4);
	CHECK(ptr2->y == 5);
	CHECK(ptr2->z == 6);

	tagged_ptr<void, 4> ptr3{ &vecs[0] };
	CHECK(ptr3 == &vecs[0]);
	ptr3 = apply_offset(ptr3.ptr(), sizeof(vec3i) * 2);
	CHECK(ptr3 == &vecs[2]);
	ptr3 = apply_offset(ptr3.ptr(), static_cast<int>(sizeof(vec3i)) * -1);
	CHECK(ptr3 == &vecs[1]);
}

MUU_POP_WARNINGS
