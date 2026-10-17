#pragma once

#include <memory>
#include <string_view>

namespace spdlog
{
	class logger;
}

namespace siv3d_template
{
	// Creates (or returns) a named console logger.
	std::shared_ptr<spdlog::logger> create_logger(std::string_view name);
}
