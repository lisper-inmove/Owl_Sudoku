#pragma once

#include <string_view>

namespace siv3d_template
{
	// Returns the application version defined by CMake.
	std::string_view version() noexcept;
}
