#include "DeathSequenceController.h"

#include "Scene.h"
#include "GameObject.h"
#include "ResourceManager.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "ServiceLocator.h"
#include "GameSounds.h"
#include "AnimationComponent.h"

#include <memory>

digger::DeathSequenceController::DeathSequenceController(
	dae::Scene* scene,
	float duration)
	: m_Scene(scene)
	, m_Duration(duration)
{
}

void digger::DeathSequenceController::Begin(const glm::vec2& tombstonePosition)
{
	if (m_Active)
		return;

	m_Active = true;
	m_Timer = 0.f;

	auto& sound = dae::ServiceLocator::GetSoundSystem();

	sound.Stop(GameSound::BackgroundMusic);
	sound.Stop(GameSound::MoneyBagWiggle);
	sound.Stop(GameSound::MoneyBagFalling);
	sound.Stop(GameSound::BulletTravel);

	sound.Play(GameSound::PlayerDeathSfx, 1.0f);
	sound.Play(GameSound::PlayerDeathMusic, 1.0f);

	SpawnTombstone(tombstonePosition);
}

bool digger::DeathSequenceController::Update(float deltaTime)
{
	if (!m_Active)
		return false;

	m_Timer += deltaTime;

	if (m_Timer < m_Duration)
		return false;

	Clear();
	return true;
}

void digger::DeathSequenceController::Clear()
{
	m_Active = false;
	m_Timer = 0.f;

	RemoveTombstone();
}

void digger::DeathSequenceController::SpawnTombstone(const glm::vec2& position)
{
	if (!m_Scene)
		return;

	RemoveTombstone();

	auto tomb = std::make_unique<dae::GameObject>();
	auto* tombPtr = tomb.get();

	auto* tr = tomb->AddComponent<dae::TransformComponent>(tombPtr);
	tr->SetLocalPosition(position.x, position.y);

	
	

	tomb->AddComponent<dae::RenderComponent>(
		tombPtr,
		dae::ResourceManager::GetInstance().LoadTexture("VGRAVE1.png"));

	auto* animation =
		tomb->AddComponent<dae::AnimationComponent>(tombPtr);


	animation->AddAnimation(
		"Death",
		{
			"VGRAVE1.png",
			"VGRAVE2.png",
			"VGRAVE3.png",
			"VGRAVE4.png",
			"VGRAVE5.png",

		},
		0.25f,
		false);

	animation->Play("Death");

	m_Tombstone = tombPtr;
	m_Scene->Add(std::move(tomb));
}

void digger::DeathSequenceController::RemoveTombstone()
{
	if (!m_Tombstone || !m_Scene)
	{
		m_Tombstone = nullptr;
		return;
	}

	m_Scene->Remove(*m_Tombstone);
	m_Tombstone = nullptr;
}