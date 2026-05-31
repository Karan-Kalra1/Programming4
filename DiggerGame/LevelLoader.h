#pragma once
#include <filesystem>
#include "LevelData.h"
#include "GameObject.h"

static constexpr int DirtSubdivisions = 16;
static constexpr int DirtPieceCount = DirtSubdivisions * DirtSubdivisions;

struct DirtTile
{

	dae::GameObject* pieces[DirtPieceCount]{};
	bool removed[DirtPieceCount]{};
};


namespace digger
{
	class LevelLoader final
	{
	public:
		static LevelData Load(const std::filesystem::path& path);
	};
}