#pragma once

#include <array>
#include <vector>

namespace dae
{
	class Scene;
	class GameObject;
	class TextComponent;
}

namespace digger
{
	class GameManagerComponent;

	class HudController final
	{
	public:
		explicit HudController(dae::Scene* scene);

		void Create(GameManagerComponent& game);
		void Update(GameManagerComponent& game);
		void Clear();

	private:
		dae::Scene* m_Scene{};

		std::vector<dae::GameObject*> m_HUDObjects{};
		std::vector<std::array<dae::GameObject*, 4>> m_PlayerLifeIcons{};
		std::vector<dae::TextComponent*> m_PlayerLifeLabels{};

		dae::TextComponent* m_ScoreText{};
	};
}