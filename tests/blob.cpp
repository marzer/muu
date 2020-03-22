#include "tests.h"
#include "../include/muu/blob.h"

static_assert(blob::default_alignment);
static_assert(blob::default_alignment == __STDCPP_DEFAULT_NEW_ALIGNMENT__);

TEST_CASE("blob")
{
	//default ctor
	blob blb;
	CHECK(blb.size() == 0);
	CHECK(blb.data() == nullptr);
	CHECK(blb.alignment() == blob::default_alignment);
	CHECK(has_single_bit(blb.alignment()));
	CHECK(!blb);

	//sized init
	blob blb2{ 1024 };
	CHECK(blb2);
	CHECK(blb2.size() == 1024);
	CHECK(blb2.data() != nullptr);
	CHECK(blb2.alignment() == blob::default_alignment);
	CHECK(has_single_bit(blb2.alignment()));

	//copy constructor (and move assignment)
	blb = blob{ blb2 };
	CHECK(blb);
	CHECK(blb.size() == 1024);
	CHECK(blb.data() != nullptr);
	CHECK(blb.alignment() == blob::default_alignment);
	CHECK(has_single_bit(blb.alignment()));
	CHECK(blb2);
	CHECK(blb2.size() == blb.size());
	CHECK(blb2.data() != nullptr);
	CHECK(blb2.data() != blb.data());
	CHECK(blb2.alignment() == blb.alignment());
	CHECK(blb2.alignment() == blob::default_alignment);
	CHECK(memcmp(blb.data(), blb2.data(), blb.size()) == 0);

	//move constructor
	blb = blob{ std::move(blb2) };
	CHECK(blb);
	CHECK(blb.size() == 1024);
	CHECK(blb.data() != nullptr);
	CHECK(blb.alignment() == blob::default_alignment);
	CHECK(has_single_bit(blb.alignment()));
	CHECK(!blb2);
	CHECK(blb2.size() == 0);
	CHECK(blb2.data() == nullptr);
	CHECK(blb2.alignment() == blob::default_alignment);

	//aligned data
	blb2 = blob{ 2048, nullptr, 128 };
	CHECK(blb2);
	CHECK(blb2.size() == 2048);
	CHECK(blb2.data() != nullptr);
	CHECK(blb2.alignment() == 128);

	//copy constructor w/ alignment change
	blb = blob{ blb2, 64 };
	CHECK(blb);
	CHECK(blb.size() == blb2.size());
	CHECK(blb.data() != nullptr);
	CHECK(blb.data() != blb2.data());
	CHECK(blb.alignment() == 64);
	CHECK(blb2);
	CHECK(memcmp(blb.data(), blb2.data(), blb.size()) == 0);
}
