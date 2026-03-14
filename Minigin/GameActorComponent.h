#pragma once
#include "Component.h"
#include "Subject.h"

namespace dae
{
	class GameActorComponent final : public Component
	{
	public:
		GameActorComponent(GameObject* owner, int lives = 3, int score = 0, int playerIndex = 0);

		using Component::GetOwner;

		int GetLives() const { return m_Lives; }
		int GetScore() const { return m_Score; }
		int GetPlayerIndex() const { return m_PlayerIndex; }

		void LoseLife();
		void AddScore(int amount);
		void SetLives(int lives);
		void SetScore(int score);

		Subject& GetSubject() { return m_Subject; }
		

	private:
		int m_Lives{ 3 };
		int m_Score{ 0 };
		int m_PlayerIndex{};

		Subject m_Subject{};
	};
}