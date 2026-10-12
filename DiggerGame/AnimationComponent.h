
#pragma once

#include "Component.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace dae
{
    class Texture2D;

    class AnimationComponent final : public Component
    {
    public:
        explicit AnimationComponent(GameObject* owner);

        bool AddAnimation(
            const std::string& name,
            const std::vector<std::string>& framePaths,
            float frameDuration,
            bool looping);

        bool Play(const std::string& name, bool restart = false);

        void Update() override;

        bool IsPlaying() const;
        const std::string& GetCurrentAnimation() const;

    private:
        struct Animation
        {
            std::vector<std::shared_ptr<Texture2D>> frames{};
            float frameDuration{ 0.1f };
            bool looping{ true };
        };

        void ApplyCurrentFrame();

        std::unordered_map<std::string, Animation> m_Animations{};

        std::string m_CurrentAnimation{};
        size_t m_CurrentFrame{};
        float m_ElapsedTime{};
        bool m_IsPlaying{};
    };
}
