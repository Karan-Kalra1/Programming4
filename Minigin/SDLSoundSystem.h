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

		SDLSoundSystem(const SDLSoundSystem&) = delete;
		SDLSoundSystem(SDLSoundSystem&&) = delete;
		SDLSoundSystem& operator=(const SDLSoundSystem&) = delete;
		SDLSoundSystem& operator=(SDLSoundSystem&&) = delete;

		void RegisterSound(SoundId id, const std::string& filePath) override;
		void Play(SoundId id, float volume = 1.0f) override;
		void StopAll() override;

	private:
		class Impl;
		std::unique_ptr<Impl> m_Impl;
	};
}