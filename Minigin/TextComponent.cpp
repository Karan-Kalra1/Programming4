
#include <stdexcept>
#include "TextComponent.h"
#include "Renderer.h"
#include "Texture2D.h"
#include "Font.h"
#include "GameObject.h"
#include "TransformComponent.h"
#include <SDL3_ttf/SDL_ttf.h>

dae::TextComponent::TextComponent(GameObject* owner,
	const std::string& text,
	std::shared_ptr<Font> font,
	const SDL_Color& color)
	: Component::Component(owner),
	m_needsUpdate(true),
	m_text(text),
	m_color(color),
	m_font(std::move(font))
	
{
}

void dae::TextComponent::Update()
{
	if (!m_needsUpdate) return;

	auto surf = TTF_RenderText_Blended(
		m_font->GetFont(),
		m_text.c_str(),
		m_text.length(),
		m_color);

	if (!surf)
		throw std::runtime_error(SDL_GetError());

	auto texture =
		SDL_CreateTextureFromSurface(
			Renderer::GetInstance().GetSDLRenderer(),
			surf);

	SDL_DestroySurface(surf);

	m_texture = std::make_shared<Texture2D>(texture);
	m_needsUpdate = false;
}

void dae::TextComponent::Render() const
{
	auto transform = GetOwner()->GetComponent<TransformComponent>();
	if (!transform || !m_texture) return;

	auto pos = transform->GetLocalPosition();
	Renderer::GetInstance().RenderTexture(*m_texture, pos.x, pos.y);
}

void dae::TextComponent::SetText(const std::string& text)
{
	m_text = text;
	m_needsUpdate = true;
}

void dae::TextComponent::SetColor(const SDL_Color& color)
{
	m_color = color;
	m_needsUpdate = true;
}

