#include "LoggingSoundSystem.h"
#include <iostream>

dae::LoggingSoundSystem::LoggingSoundSystem(std::unique_ptr<SoundSystem> wrapped)
	: m_Wrapped(std::move(wrapped))
{
}

void dae::LoggingSoundSystem::RegisterSound(SoundId id, const std::string& filePath)
{
	std::cout << "[Sound] Register " << id << " -> " << filePath << '\n';
	m_Wrapped->RegisterSound(id, filePath);
}

void dae::LoggingSoundSystem::Play(SoundId id, float volume)
{
	std::cout << "[Sound] Play " << id << " volume " << volume << '\n';
	m_Wrapped->Play(id, volume);
}

void dae::LoggingSoundSystem::StopAll()
{
	std::cout << "[Sound] StopAll\n";
	m_Wrapped->StopAll();
}

void dae::LoggingSoundSystem::SetMuted(bool muted)
{
	std::cout << "[Sound] Muted: " << std::boolalpha << muted << '\n';

	if (m_Wrapped)
		m_Wrapped->SetMuted(muted);
}

bool dae::LoggingSoundSystem::IsMuted() const
{
	return m_Wrapped ? m_Wrapped->IsMuted() : true;
}