#pragma once

namespace dae
{
	class GameObject;

	class Component
	{
	public:

		Component(GameObject* owner) : m_pOwner( owner ) {}
		virtual ~Component() = default;
		virtual void Update() {}
		virtual void Render() const {}


	protected:
		GameObject* GetOwner() const { return m_pOwner; }

	private:
		GameObject* m_pOwner{};
	};
}
