#include "MoneyBagComponent.h"

#include "GameManagerComponent.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "ResourceManager.h"
#include "MiniginTime.h"
#include "EnemyComponent.h"
#include "GridMovementComponent.h"
#include "ServiceLocator.h"
#include "GameSounds.h"


#include <glm/geometric.hpp>
#include <algorithm>

digger::MoneyBagComponent::MoneyBagComponent(
	dae::GameObject* owner,
	GameManagerComponent* manager,
	int tileSize,
	glm::vec2 offset)
	: Component(owner)
	, m_Manager(manager)
	, m_TileSize(tileSize)
	, m_Offset(offset)
{
}

void digger::MoneyBagComponent::Update()
{
	if (m_Manager && m_Manager->IsGameplayFrozen())
		return;

	switch (m_State)
	{
	case MoneyBagState::Stable:
		UpdateStable();
		break;

	case MoneyBagState::WaitingToFall:
		UpdateWaitingToFall();
		break;

	case MoneyBagState::Falling:
		UpdateFalling();
		break;

	case MoneyBagState::BrokenGold:
	{
		if (!m_Manager)
			return;

		auto* tr = GetOwner()->GetComponent<dae::TransformComponent>();
		if (!tr)
			return;

		const auto& pos = tr->GetWorldPosition();

		glm::vec2 goldCenter{
			pos.x + 32.f,
			pos.y + 32.f
		};

		const int playerIndex =
			m_Manager->GetDiggerPlayerIndexAtWorldPosition(
				goldCenter,
				m_Manager->GetCollisionRadius());

		if (playerIndex != -1)
			m_Manager->CollectGold(GetOwner());

		break;
	}
	}
}

void digger::MoneyBagComponent::SetGridPosition(const glm::ivec2& pos)
{
	m_GridPosition = pos;
	m_WorldPosition = GridToWorld(pos);
	m_TargetWorldPosition = m_WorldPosition;

	if (auto* tr = GetOwner()->GetComponent<dae::TransformComponent>())
		tr->SetLocalPosition(m_WorldPosition.x, m_WorldPosition.y);
}

glm::vec2 digger::MoneyBagComponent::GridToWorld(const glm::ivec2& grid) const
{
	return {
		m_Offset.x + static_cast<float>(grid.x * m_TileSize),
		m_Offset.y + static_cast<float>(grid.y * m_TileSize)
	};
}

void digger::MoneyBagComponent::SetState(MoneyBagState state)
{
	if (m_State == state)
		return;

	auto& sound = dae::ServiceLocator::GetSoundSystem();

	if (m_State == MoneyBagState::WaitingToFall)
		sound.Stop(GameSound::MoneyBagWiggle);

	if (m_State == MoneyBagState::Falling)
		sound.Stop(GameSound::MoneyBagFalling);

	m_State = state;

	if (m_State == MoneyBagState::WaitingToFall)
	{
		sound.PlayLooping(GameSound::MoneyBagWiggle, 0.8f);
	}
	else if (m_State == MoneyBagState::Falling)
	{
		sound.Stop(GameSound::MoneyBagWiggle);
		sound.PlayLooping(GameSound::MoneyBagFalling, 0.9f);
	}
	else if (m_State == MoneyBagState::Stable ||
		m_State == MoneyBagState::BrokenGold)
	{
		sound.Stop(GameSound::MoneyBagWiggle);
		sound.Stop(GameSound::MoneyBagFalling);
	}
}

void digger::MoneyBagComponent::UpdateStable()
{
	if (!m_Manager)
		return;

	const glm::ivec2 below{
		m_GridPosition.x,
		m_GridPosition.y + 1
	};

	const bool hasSupport =
		m_Manager->IsSolidForMoneyBag({ m_GridPosition.x, m_GridPosition.y + 1 });
	const bool playerUnderBag =
		m_Manager->GetPlayerIndexAtGridPosition(below) != -1;

	if (!hasSupport && !playerUnderBag)
	{
		m_FallDelayTimer = 0.f;
		SetState(MoneyBagState::WaitingToFall);
	}
}

void digger::MoneyBagComponent::UpdateWaitingToFall()
{
	if (!m_Manager)
		return;

	const glm::ivec2 below{
		m_GridPosition.x,
		m_GridPosition.y + 1
	};

	const bool hasSupport =
		m_Manager->IsSolidForMoneyBag({ m_GridPosition.x, m_GridPosition.y + 1 });
	const bool playerUnderBag =
		m_Manager->GetPlayerIndexAtGridPosition(below) != -1;

	if (hasSupport || playerUnderBag)
	{
		m_FallDelayTimer = 0.f;
		SetState(MoneyBagState::Stable);
		return;
	}

	m_FallDelayTimer += dae::MiniginTime::GetDeltaTime();

	if (m_FallDelayTimer >= m_FallDelay)
	{
		m_FallDistance = 0;
		m_TargetWorldPosition =
			GridToWorld(m_GridPosition + glm::ivec2{ 0, 1 });

		SetState(MoneyBagState::Falling);
	}
}

void digger::MoneyBagComponent::UpdateFalling()
{
	const float dt = dae::MiniginTime::GetDeltaTime();

	glm::vec2 toTarget = m_TargetWorldPosition - m_WorldPosition;
	const float distance = glm::length(toTarget);
	const float moveDistance = m_FallSpeed * dt;

	bool landed = false;

	if (distance <= moveDistance)
	{
		m_WorldPosition = m_TargetWorldPosition;
		m_GridPosition.y += 1;
		++m_FallDistance;
		m_HasFallen = true;

		const glm::ivec2 below{
			m_GridPosition.x,
			m_GridPosition.y + 1
		};

		if (!m_Manager || m_Manager->IsSolidForMoneyBag(below))
		{
			landed = true;
		}
		else
		{
			m_TargetWorldPosition =
				GridToWorld(m_GridPosition + glm::ivec2{ 0, 1 });
		}
	}
	else
	{
		m_WorldPosition += glm::normalize(toTarget) * moveDistance;
	}

	// Move bag visually first
	if (auto* tr = GetOwner()->GetComponent<dae::TransformComponent>())
		tr->SetLocalPosition(m_WorldPosition.x, m_WorldPosition.y);

	// Detect victims after the bag moved.
	CheckCrushTargets();

	// Drag victims before killing or releasing them.
	DragTargetsDown();

	if (landed)
	{
		if (m_FallDistance >= 2)
		{
			KillDraggedTargets();
			BreakIntoGold();
		}
		else
		{
			ReleaseDraggedTargets();
			SetState(MoneyBagState::Stable);
		}
	}
}

bool digger::MoneyBagComponent::TryPush(const glm::ivec2& direction)
{
	if (m_State != MoneyBagState::Stable)
		return false;

	if (direction.y != 0)
		return false;

	if (!m_HasFallen && m_Manager && m_Manager->HasDirtBelow(m_GridPosition))
		return false;

	const glm::ivec2 target = m_GridPosition + direction;

	if (!m_Manager || !m_Manager->CanMoneyBagMoveTo(target))
		return false;

	SetGridPosition(target);
	return true;
}

void digger::MoneyBagComponent::BreakIntoGold()
{
	SetState(MoneyBagState::BrokenGold);

	auto* render = GetOwner()->GetComponent<dae::RenderComponent>();
	if (render)
	{
		render->SetTexture(
			dae::ResourceManager::GetInstance().LoadTexture("Gold.png"));
	}
}

void digger::MoneyBagComponent::UpdateVisual()
{
}

void digger::MoneyBagComponent::CheckCrushTargets()
{
	if (!m_Manager)
		return;

	auto* bagTr = GetOwner()->GetComponent<dae::TransformComponent>();
	if (!bagTr)
		return;

	const auto& bagPos3 = bagTr->GetWorldPosition();

	glm::vec2 crushPoint{
		bagPos3.x + 32.f,
		bagPos3.y + 48.f
	};

	const float crushRadius = 34.f;

	if (m_DraggedPlayerIndex == -1)
	{
		m_DraggedPlayerIndex =
			m_Manager->GetPlayerIndexAtWorldPosition(
				crushPoint,
				crushRadius);
	}

	const auto enemies =
		m_Manager->GetEnemiesAtWorldPosition(
			crushPoint,
			crushRadius);

	for (auto* enemy : enemies)
	{
		if (!enemy)
			continue;

		const bool alreadyDragged =
			std::find(
				m_DraggedEnemies.begin(),
				m_DraggedEnemies.end(),
				enemy) != m_DraggedEnemies.end();

		if (!alreadyDragged)
		{
			m_DraggedEnemies.push_back(enemy);
			enemy->SetCrushedByBag(true);
		}
	}
}

void digger::MoneyBagComponent::DragTargetsDown()
{
	auto* bagTr = GetOwner()->GetComponent<dae::TransformComponent>();
	if (!bagTr)
		return;

	const auto& bagPos3 = bagTr->GetWorldPosition();

	if (m_DraggedPlayerIndex != -1 && m_Manager)
	{
		auto* player =
			m_Manager->GetPlayer(m_DraggedPlayerIndex);

		if (player)
		{
			if (auto* movement = player->GetComponent<GridMovementComponent>())
				movement->ReleaseAllDirections();

			if (auto* tr = player->GetComponent<dae::TransformComponent>())
				tr->SetLocalPosition(bagPos3.x, bagPos3.y);
		}
	}

	for (auto* enemy : m_DraggedEnemies)
	{
		if (!enemy || enemy->IsDead())
			continue;

		auto* enemyObj = enemy->GetGameObject();
		if (!enemyObj)
			continue;

		if (auto* tr = enemyObj->GetComponent<dae::TransformComponent>())
			tr->SetLocalPosition(bagPos3.x, bagPos3.y);
	}
}

void digger::MoneyBagComponent::KillDraggedTargets()
{
	if (m_DraggedPlayerIndex != -1 && m_Manager)
	{
		m_Manager->DamagePlayer(m_DraggedPlayerIndex);
		m_DraggedPlayerIndex = -1;
	}

	for (auto* enemy : m_DraggedEnemies)
	{
		if (!enemy || enemy->IsDead())
			continue;

		enemy->SetCrushedByBag(false);
		enemy->Kill();
		
	}

	m_DraggedEnemies.clear();
}

void digger::MoneyBagComponent::ReleaseDraggedTargets()
{
	m_DraggedPlayerIndex = -1;

	for (auto* enemy : m_DraggedEnemies)
	{
		if (!enemy || enemy->IsDead())
			continue;

		const glm::ivec2 releaseTile =
			FindReleaseTileForEnemy(enemy);

		// If there is nowhere safe to release, kill it to avoid permanent stuck state.
		if (releaseTile == m_GridPosition)
		{
			enemy->Kill();
			continue;
		}

		enemy->SetGridPosition(releaseTile);

		enemy->SetCrushedByBag(false);
	}

	m_DraggedEnemies.clear();
}

dae::GameObject* digger::MoneyBagComponent::GetGameObject() const
{
	return GetOwner();
}


glm::ivec2 digger::MoneyBagComponent::FindReleaseTileForEnemy(
	EnemyComponent* enemy) const
{
	if (!m_Manager || !enemy)
		return m_GridPosition;

	static constexpr glm::ivec2 directions[]
	{
		{ -1, 0 },
		{ 1, 0 },
		{ 0, -1 },
		{ 0, 1 }
	};

	for (const auto& dir : directions)
	{
		const glm::ivec2 candidate = m_GridPosition + dir;

		if (m_Manager->HasMoneyBagAt(candidate))
			continue;

		if (m_Manager->HasEnemyAt(candidate))
			continue;

		if (m_Manager->GetPlayerIndexAtGridPosition(candidate) != -1)
			continue;

		// Use false here so the enemy is released only into an already-open tile.
		if (!m_Manager->CanEnemyMoveTo(candidate, false))
			continue;

		return candidate;
	}

	// If there is no safe tile, return current bag tile.
	
	return m_GridPosition;
}