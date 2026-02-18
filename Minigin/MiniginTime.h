#pragma once

namespace dae
{
	class MiniginTime
	{
	public:
		static void SetDeltaTime(float dt) { m_DeltaTime = dt; }
		static float GetDeltaTime() { return m_DeltaTime; }

	private:
		inline static float m_DeltaTime{};
	};
}
