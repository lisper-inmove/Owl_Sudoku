#include <siv3d_template/version.h>

#ifndef SIV3D_TEMPLATE_VERSION
#	define SIV3D_TEMPLATE_VERSION "0.0.0"
#endif

namespace siv3d_template
{
	std::string_view version() noexcept
	{
		return SIV3D_TEMPLATE_VERSION;
	}
}
