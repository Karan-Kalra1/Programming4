#pragma once
#include <string>

namespace dae
{
	using SoundId = unsigned short;

	class SoundSystem
	{
	public:
		virtual ~SoundSystem() = default;

		virtual void RegisterSound(SoundId id, const std::string& filePath) = 0;
		virtual void Play(SoundId id, float volume = 1.0f) = 0;
		virtual void StopAll() = 0;

		virtual void SetMuted(bool muted) = 0;
		virtual bool IsMuted() const = 0;
	};
}