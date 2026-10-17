#include <siv3d_template/config.h>

#include <yaml-cpp/yaml.h>

namespace siv3d_template
{
	Config Config::load(const std::filesystem::path& path)
	{
		const YAML::Node root = YAML::LoadFile(path.string());

		Config config;

		if (const auto title = root["title"])
		{
			config.title = title.as<std::string>();
		}

		if (const auto window = root["window"])
		{
			if (const auto width = window["width"])
			{
				config.windowWidth = width.as<int>();
			}

			if (const auto height = window["height"])
			{
				config.windowHeight = height.as<int>();
			}
		}

		return config;
	}
}
