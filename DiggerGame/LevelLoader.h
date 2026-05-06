#pragma once
#include <filesystem>
#include "LevelData.h"

namespace digger
{
	class LevelLoader final
	{
	public:
		static LevelData Load(const std::filesystem::path& path);
	};
}