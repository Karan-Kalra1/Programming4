#pragma once
#include "Command.h"
#include "ServiceLocator.h"

namespace dae
{
	class PlaySoundCommand final : public Command
	{
	public:
		PlaySoundCommand(SoundId id, float volume)
			: m_Id(id)
			, m_Volume(volume)
		{
		}

		void Execute() override
		{
			ServiceLocator::GetSoundSystem().Play(m_Id, m_Volume);
		}

	private:
		SoundId m_Id{};
		float m_Volume{};
	};
}