#include <siv3d_template/config.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

TEST(Config, KeepsDefaultsForMissingKeys)
{
	const siv3d_template::Config config;

	EXPECT_EQ(config.title, "Siv3D Template");
	EXPECT_EQ(config.windowWidth, 1280);
	EXPECT_EQ(config.windowHeight, 720);
}

TEST(Config, LoadsValuesFromYaml)
{
	const auto path = std::filesystem::path(::testing::TempDir()) / "siv3d_template_config_test.yaml";

	{
		std::ofstream file(path);
		ASSERT_TRUE(file.is_open());
		file << "title: Test Title\n";
		file << "window:\n";
		file << "  width: 800\n";
		file << "  height: 600\n";
	}

	const auto config = siv3d_template::Config::load(path);
	std::filesystem::remove(path);

	EXPECT_EQ(config.title, "Test Title");
	EXPECT_EQ(config.windowWidth, 800);
	EXPECT_EQ(config.windowHeight, 600);
}
