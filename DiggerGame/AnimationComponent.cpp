
#include "AnimationComponent.h"

#include "GameObject.h"
#include "RenderComponent.h"
#include "ResourceManager.h"
#include "Texture2D.h"
#include "MiniginTime.h"

#include <iostream>
#include <utility>

dae::AnimationComponent::AnimationComponent(dae::GameObject* owner)
    : Component(owner)
{}

bool dae::AnimationComponent::AddAnimation(
    const std::string& name,
    const std::vector<std::string>& framePaths,
    float frameDuration,
    bool looping)
{
    if (name.empty() || framePaths.empty() || frameDuration <= 0.f)
        return false;

    Animation animation{};
    animation.frameDuration = frameDuration;
    animation.looping = looping;

    for (const auto& path : framePaths)
    {
        auto texture =
            ResourceManager::GetInstance().LoadTexture(path);

        if (!texture)
        {
            std::cerr << "Failed to load animation frame: "
                << path << '\n';
            return false;
        }

        animation.frames.push_back(std::move(texture));
    }

    m_Animations.insert_or_assign(name, std::move(animation));

    return true;
}

bool dae::AnimationComponent::Play(
    const std::string& name,
    bool restart)
{

    const auto it = m_Animations.find(name);

    if (it == m_Animations.end() || it->second.frames.empty())
        return false;

    if (!restart && m_CurrentAnimation == name && m_IsPlaying)
        return true;

    m_CurrentAnimation = name;
    m_CurrentFrame = 0;
    m_ElapsedTime = 0.f;
    m_IsPlaying = true;

    ApplyCurrentFrame();

    return true;
}

void dae::AnimationComponent::Update()
{
    if (!m_IsPlaying)
        return;

    const auto it = m_Animations.find(m_CurrentAnimation);

    if (it == m_Animations.end())
    {
        m_IsPlaying = false;
        return;
    }

    const Animation& animation = it->second;

    m_ElapsedTime += MiniginTime::GetDeltaTime();

   
    while (m_ElapsedTime >= animation.frameDuration && m_IsPlaying)
    {
        m_ElapsedTime -= animation.frameDuration;

        if (m_CurrentFrame + 1 < animation.frames.size())
        {
            ++m_CurrentFrame;
        }
        else if (animation.looping)
        {
            m_CurrentFrame = 0;
        }
        else
        {
           
            m_CurrentFrame = animation.frames.size() - 1;
            m_IsPlaying = false;
        }

        ApplyCurrentFrame();
    }
}

void dae::AnimationComponent::ApplyCurrentFrame()
{
    const auto it = m_Animations.find(m_CurrentAnimation);

    if (it == m_Animations.end() ||
        m_CurrentFrame >= it->second.frames.size())
        return;

    auto* render =
        GetOwner()->GetComponent<RenderComponent>();

    if (!render)
        return;

    render->SetTexture(it->second.frames[m_CurrentFrame]);
}

bool dae::AnimationComponent::IsPlaying() const
{
    return m_IsPlaying;
}

const std::string&
dae::AnimationComponent::GetCurrentAnimation() const
{
    return m_CurrentAnimation;
}
