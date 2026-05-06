#pragma once
#include <memory>
#include "SoundSystem.h"

namespace dae
{
	class NullSoundSystem final : public SoundSystem
	{
	public:
		void RegisterSound(SoundId, const std::string&) override {}
		void Play(SoundId, float) override {}
		void StopAll() override {}
		void SetMuted(bool muted) override { m_Muted = muted; }
		bool IsMuted() const override { return m_Muted; }

	private:
		bool m_Muted{};
	};

	class ServiceLocator final
	{
	public:
		static SoundSystem& GetSoundSystem()
		{
			return *m_SoundSystem;
		}

		static void RegisterSoundSystem(std::unique_ptr<SoundSystem> soundSystem)
		{
			m_SoundSystem = soundSystem
				? std::move(soundSystem)
				: std::make_unique<NullSoundSystem>();
		}

	private:
		inline static std::unique_ptr<SoundSystem> m_SoundSystem =
			std::make_unique<NullSoundSystem>();
	};
}