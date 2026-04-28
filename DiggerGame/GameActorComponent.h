#pragma once
#include "Component.h"
#include "Observer.h"
#include "Subject.h"

namespace dae
{
	class GameActorComponent final : public Component, public Observer
	{
	public:
		GameActorComponent(GameObject* owner, int lives = 3, int score = 0, int playerIndex = 0);

		int GetLives() const { return m_Lives; }
		int GetScore() const { return m_Score; }
		int GetPlayerIndex() const { return m_PlayerIndex; }

		void OnNotify(const Event& event) override;

		Subject& GetSubject() { return m_Subject; }

		using Component::GetOwner;

	private:
		void LoseLife(int amount);
		void AddScore(int amount);

		int m_Lives{ 3 };
		int m_Score{ 0 };
		int m_PlayerIndex{};

		Subject m_Subject{};
	};
}