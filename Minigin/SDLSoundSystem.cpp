#include "SDLSoundSystem.h"

#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>

namespace dae
{
	class SDLSoundSystem::Impl final
	{
	public:
		Impl()
		{
			if (!MIX_Init())
			{
				std::cerr << "MIX_Init failed: " << SDL_GetError() << '\n';
			}

			m_Mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
			if (!m_Mixer)
			{
				std::cerr << "MIX_CreateMixerDevice failed: " << SDL_GetError() << '\n';
			}

			m_Worker = std::thread(&Impl::ProcessRequests, this);
		}

		~Impl()
		{
			Enqueue(Request{ RequestType::Quit });

			if (m_Worker.joinable())
				m_Worker.join();

			for (auto& [id, track] : m_Tracks)
			{
				(void)id;

				if (track)
					MIX_DestroyTrack(track);
			}

			m_Tracks.clear();

			for (auto& [id, audio] : m_LoadedSounds)
			{
				(void)id;

				if (audio)
					MIX_DestroyAudio(audio);
			}

			m_LoadedSounds.clear();

			if (m_Mixer)
			{
				MIX_DestroyMixer(m_Mixer);
				m_Mixer = nullptr;
			}

			MIX_Quit();
		}

		void RegisterSound(SoundId id, const std::string& filePath)
		{
			Enqueue(Request{ RequestType::Register, id, 1.0f, filePath });
		}

		void Play(SoundId id, float volume)
		{
			if (m_Muted.load())
				return;

			volume = std::clamp(volume, 0.0f, 1.0f);
			Enqueue(Request{ RequestType::Play, id, volume, {} });
		}

		void PlayLooping(SoundId id, float volume)
		{
			if (m_Muted.load())
				return;

			volume = std::clamp(volume, 0.0f, 1.0f);
			Enqueue(Request{ RequestType::PlayLooping, id, volume, {} });
		}

		void Stop(SoundId id)
		{
			Enqueue(Request{ RequestType::Stop, id, 1.0f, {} });
		}

		void StopAll()
		{
			Enqueue(Request{ RequestType::StopAll });
		}

		void SetMuted(bool muted)
		{
			m_Muted.store(muted);

			if (muted)
				StopAll();
		}

		bool IsMuted() const
		{
			return m_Muted.load();
		}

	private:
		enum class RequestType
		{
			Register,
			Play,
			PlayLooping,
			Stop,
			StopAll,
			Quit
		};

		struct Request
		{
			RequestType type{};
			SoundId id{};
			float volume{};
			std::string filePath{};
		};

		void Enqueue(Request request)
		{
			{
				std::lock_guard lock{ m_Mutex };
				m_Requests.push(std::move(request));
			}

			m_Condition.notify_one();
		}

		void ProcessRequests()
		{
			while (true)
			{
				Request request{};

				{
					std::unique_lock lock{ m_Mutex };
					m_Condition.wait(lock, [this]
						{
							return !m_Requests.empty();
						});

					request = std::move(m_Requests.front());
					m_Requests.pop();
				}

				if (request.type == RequestType::Quit)
					break;

				switch (request.type)
				{
				case RequestType::Register:
					m_SoundPaths[request.id] = request.filePath;
					break;

				case RequestType::Play:
					PlayInternal(request.id, request.volume);
					break;

				case RequestType::PlayLooping:
					PlayLoopingInternal(request.id, request.volume);
					break;

				case RequestType::Stop:
					StopInternal(request.id);
					break;

				case RequestType::StopAll:
					StopAllInternal();
					break;

				case RequestType::Quit:
					break;
				}
			}
		}

		void PlayInternal(SoundId id, float volume)
		{
			if (m_Muted.load())
				return;

			MIX_Audio* audio = GetOrLoad(id);
			if (!audio || !m_Mixer)
				return;

			// Fire-and-forget one-shot sound.
			// SDL3_mixer's MIX_PlayAudio does not expose a track to stop later.
			(void)volume;

			if (!MIX_PlayAudio(m_Mixer, audio))
			{
				std::cerr << "MIX_PlayAudio failed: " << SDL_GetError() << '\n';
			}
		}

		void PlayLoopingInternal(SoundId id, float volume)
		{
			if (m_Muted.load())
				return;

			MIX_Audio* audio = GetOrLoad(id);
			if (!audio || !m_Mixer)
				return;

			MIX_Track* track = GetOrCreateTrack(id);
			if (!track)
				return;

			if (!MIX_SetTrackAudio(track, audio))
			{
				std::cerr << "MIX_SetTrackAudio failed: " << SDL_GetError() << '\n';
				return;
			}

			MIX_SetTrackGain(track, volume);

			SDL_PropertiesID props = SDL_CreateProperties();
			SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);

			if (!MIX_PlayTrack(track, props))
			{
				std::cerr << "MIX_PlayTrack failed: " << SDL_GetError() << '\n';
			}

			SDL_DestroyProperties(props);
		}

		void StopInternal(SoundId id)
		{
			auto it = m_Tracks.find(id);
			if (it == m_Tracks.end() || !it->second)
				return;

			if (!MIX_StopTrack(it->second, 0))
			{
				std::cerr << "MIX_StopTrack failed: " << SDL_GetError() << '\n';
			}
		}

		void StopAllInternal()
		{
			if (m_Mixer)
				MIX_StopAllTracks(m_Mixer, 0);
		}

		MIX_Audio* GetOrLoad(SoundId id)
		{
			if (auto it = m_LoadedSounds.find(id); it != m_LoadedSounds.end())
				return it->second;

			auto pathIt = m_SoundPaths.find(id);
			if (pathIt == m_SoundPaths.end())
			{
				std::cerr << "Sound id not registered: " << id << '\n';
				return nullptr;
			}

			MIX_Audio* audio = MIX_LoadAudio(m_Mixer, pathIt->second.c_str(), true);
			if (!audio)
			{
				std::cerr << "MIX_LoadAudio failed for " << pathIt->second
					<< ": " << SDL_GetError() << '\n';
				return nullptr;
			}

			m_LoadedSounds[id] = audio;
			return audio;
		}

		MIX_Track* GetOrCreateTrack(SoundId id)
		{
			if (auto it = m_Tracks.find(id); it != m_Tracks.end())
				return it->second;

			if (!m_Mixer)
				return nullptr;

			MIX_Track* track = MIX_CreateTrack(m_Mixer);
			if (!track)
			{
				std::cerr << "MIX_CreateTrack failed: " << SDL_GetError() << '\n';
				return nullptr;
			}

			m_Tracks[id] = track;
			return track;
		}

		std::unordered_map<SoundId, std::string> m_SoundPaths{};
		std::unordered_map<SoundId, MIX_Audio*> m_LoadedSounds{};
		std::unordered_map<SoundId, MIX_Track*> m_Tracks{};

		MIX_Mixer* m_Mixer{};

		std::queue<Request> m_Requests{};
		std::mutex m_Mutex{};
		std::condition_variable m_Condition{};
		std::thread m_Worker{};

		std::atomic_bool m_Muted{ false };
	};

	SDLSoundSystem::SDLSoundSystem()
		: m_Impl(std::make_unique<Impl>())
	{
	}

	SDLSoundSystem::~SDLSoundSystem() = default;

	void SDLSoundSystem::RegisterSound(SoundId id, const std::string& filePath)
	{
		m_Impl->RegisterSound(id, filePath);
	}

	void SDLSoundSystem::Play(SoundId id, float volume)
	{
		m_Impl->Play(id, volume);
	}

	void SDLSoundSystem::PlayLooping(SoundId id, float volume)
	{
		m_Impl->PlayLooping(id, volume);
	}

	void SDLSoundSystem::Stop(SoundId id)
	{
		m_Impl->Stop(id);
	}

	void SDLSoundSystem::StopAll()
	{
		m_Impl->StopAll();
	}

	void SDLSoundSystem::SetMuted(bool muted)
	{
		m_Impl->SetMuted(muted);
	}

	bool SDLSoundSystem::IsMuted() const
	{
		return m_Impl->IsMuted();
	}
}