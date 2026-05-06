#pragma once
#include <memory>
#include "SoundSystem.h"

namespace dae
{
	class LoggingSoundSystem final : public SoundSystem
	{
	public:
		explicit LoggingSoundSystem(std::unique_ptr<SoundSystem> wrapped);

		void RegisterSound(SoundId id, const std::string& filePath) override;
		void Play(SoundId id, float volume = 1.0f) override;
		void StopAll() override;
		void SetMuted(bool muted) override;
		bool IsMuted() const override;

	private:
		std::unique_ptr<SoundSystem> m_Wrapped;
	};
}