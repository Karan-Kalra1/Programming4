#pragma once

namespace dae
{
	class GameObject;

	class Component
	{
	public:
		virtual ~Component() = default;

		virtual void Update() {}
		virtual void Render() const {}

		void SetOwner(GameObject* owner) { m_pOwner = owner; }

	protected:
		GameObject* GetOwner() const { return m_pOwner; }

	private:
		GameObject* m_pOwner{};
	};
}
