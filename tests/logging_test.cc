#include <siv3d_template/logging.h>

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

TEST(Logging, CreatesNamedLogger)
{
	const auto logger = siv3d_template::create_logger("siv3d_template_test_logger");

	ASSERT_NE(logger, nullptr);
	EXPECT_EQ(logger->name(), "siv3d_template_test_logger");
}

TEST(Logging, ReturnsExistingLogger)
{
	const auto first = siv3d_template::create_logger("siv3d_template_test_logger_shared");
	const auto second = siv3d_template::create_logger("siv3d_template_test_logger_shared");

	EXPECT_EQ(first, second);
}
