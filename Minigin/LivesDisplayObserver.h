#pragma once
#include <string>
#include "Observer.h"

namespace dae
{
	class TextComponent;
	class GameActorComponent;

	class LivesDisplayObserver final : public Observer
	{
	public:
		LivesDisplayObserver(GameActorComponent* actor, TextComponent* text, const std::string& prefix = "Lives: ");

		void StartListening();
		void StopListening();

		void OnNotify(const Event& event) override;

	private:
		void Refresh();

		GameActorComponent* m_pActor{};
		TextComponent* m_pText{};
		std::string m_Prefix{};
	};
}