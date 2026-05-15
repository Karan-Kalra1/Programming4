#pragma once
#include <filesystem>
#include "LevelData.h"
#include "GameObject.h"


struct DirtTile
{
	static constexpr int Size = 16;

	dae::GameObject* pieces[Size]{};
	bool removed[Size]{};
};


namespace digger
{
	class LevelLoader final
	{
	public:
		static LevelData Load(const std::filesystem::path& path);
	};
}