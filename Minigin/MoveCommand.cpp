#include "MoveCommand.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include "MiniginTime.h"

dae::MoveCommand::MoveCommand(GameObject* gameObject, const glm::vec2& direction, float speed)
	: m_pGameObject(gameObject)
	, m_Direction(direction)
	, m_Speed(speed)
{
}

void dae::MoveCommand::Execute()
{
	if (!m_pGameObject)
		return;

	auto transform = m_pGameObject->GetComponent<TransformComponent>();
	if (!transform)
		return;

	const auto pos = transform->GetLocalPosition();
	transform->SetLocalPosition(
		pos.x + m_Direction.x * m_Speed * dae::MiniginTime::GetDeltaTime(),
		pos.y + m_Direction.y * m_Speed * dae::MiniginTime::GetDeltaTime(),
		pos.z);
}