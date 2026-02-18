#pragma once
#include <sstream>
#include <iomanip>
#include "Component.h"
#include "MiniginTime.h"
#include "TextComponent.h"
#include <string>
#include  "GameObject.h"

namespace dae
{
	class FPSComponent final : public Component
	{
	public:
		void Update() override
		{
			m_time += MiniginTime::GetDeltaTime();
			m_frames++;

			if (m_time >= 1.f)
			{
				auto text = GetOwner()->GetComponent<TextComponent>();
				if (text)
				{
					float fps = static_cast<float>(m_frames) / m_time;

					std::ostringstream ss;
					ss << std::fixed << std::setprecision(1) << fps;

					text->SetText("FPS: " + ss.str());
				}

				m_frames = 0;
				m_time = 0.f;
			}
		}

	private:
		float m_time{};
		int m_frames{};
	};
}
