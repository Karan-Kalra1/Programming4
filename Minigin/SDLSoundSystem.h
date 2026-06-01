#pragma once
#include <memory>
#include "SoundSystem.h"


namespace dae
{
	class SDLSoundSystem final : public SoundSystem
	{
	public:
		SDLSoundSystem();
		~SDLSoundSystem() override;

		void RegisterSound(SoundId id, const std::string& filePath) override;

		void Play(SoundId id, float volume = 1.0f) override;
		void PlayLooping(SoundId id, float volume = 1.0f) override;

		void Stop(SoundId id) override;
		void StopAll() override;

		void SetMuted(bool muted) override;
		bool IsMuted() const override;

	private:
		class Impl;
		std::unique_ptr<Impl> m_Impl;
	};
}