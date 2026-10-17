#pragma once

#include <filesystem>
#include <string>

namespace siv3d_template
{
	struct Config
	{
		std::string title = "Siv3D Template";
		int windowWidth = 1280;
		int windowHeight = 720;

		// Loads configuration from a YAML file. Missing keys keep their defaults.
		static Config load(const std::filesystem::path& path);
	};
}
