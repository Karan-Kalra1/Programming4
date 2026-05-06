#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace digger
{
	struct LevelData
	{
		int width{};
		int height{};

		std::vector<std::string> tiles;

		glm::ivec2 playerSpawn{};
		std::vector<glm::ivec2> diamonds;
		std::vector<glm::ivec2> enemies;
	};
}