#include "tests.h"
#include "../include/muu/hashing.h"

TEST_CASE("hashing - sha1")
{
	static constexpr auto sha1_test = [](std::string_view input, std::string_view expected) noexcept
	{
		sha1 hasher;
		hasher(input);
		hasher.finish();
		std::ostringstream oss;
		oss << hasher;
		CHECK(oss.str() == expected);
	};

	sha1_test("The quick brown fox jumps over the lazy dog"sv, "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12"sv);
}

