#include <siv3d_template/version.h>

#include <gtest/gtest.h>

TEST(Version, IsNotEmpty)
{
	EXPECT_FALSE(siv3d_template::version().empty());
}
