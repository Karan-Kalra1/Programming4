#include "LevelLoader.h"
#include <fstream>
#include <stdexcept>

digger::LevelData digger::LevelLoader::Load(const std::filesystem::path& path)
{
	std::ifstream file{ path };
	if (!file)
		throw std::runtime_error("Failed to open level file: " + path.string());

	LevelData data{};
	std::string line{};

	while (std::getline(file, line))
	{
		if (line.empty())
			continue;

		data.tiles.push_back(line);
	}

	data.height = static_cast<int>(data.tiles.size());
	data.width = data.height > 0 ? static_cast<int>(data.tiles[0].size()) : 0;

	for (int y{}; y < data.height; ++y)
	{
		for (int x{}; x < data.width; ++x)
		{
			const char c = data.tiles[y][x];

			if (c == 'P')
				data.playerSpawn = { x, y };
			else if (c == 'D')
				data.diamonds.push_back({ x, y });
			else if (c == 'M')
				data.enemies.push_back({ x, y });
		}
	}

	return data;
}