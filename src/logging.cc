#include <siv3d_template/logging.h>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <string>

namespace siv3d_template
{
	std::shared_ptr<spdlog::logger> create_logger(std::string_view name)
	{
		const std::string loggerName(name);

		if (const auto existing = spdlog::get(loggerName))
		{
			return existing;
		}

		auto logger = spdlog::stdout_color_mt(loggerName);
		logger->set_level(spdlog::level::info);
		return logger;
	}
}
