#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
#include <vld.h>
#endif

#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "TextComponent.h"
#include "TransformComponent.h"
#include "RenderComponent.h"
#include "FPSComponent.h"
#include "TextComponent.h"
#include "Texture2D.h"
#include "CircularMovementComponent.h"
#include "Scene.h"

#include <filesystem>
namespace fs = std::filesystem;

static void load()
{
	
	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	
	auto font =
		dae::ResourceManager::GetInstance()
		.LoadFont("Lingua.otf", 36);

	
	{
		auto go = std::make_unique<dae::GameObject>();

		auto tex =
			dae::ResourceManager::GetInstance()
			.LoadTexture("background.png");

		

		auto transform =
			go->AddComponent<dae::TransformComponent>(go.get());
		transform->SetLocalPosition(0, 0);

		go->AddComponent<dae::RenderComponent>(go.get(),tex);

		scene.Add(std::move(go));
	}

	
	{
		auto go = std::make_unique<dae::GameObject>();

		auto transform =
			go->AddComponent<dae::TransformComponent>(go.get());
		
		auto tex =
			dae::ResourceManager::GetInstance()
			.LoadTexture("logo.png");

		

		transform->SetLocalPosition(358, 180);


		go->AddComponent<dae::RenderComponent>(go.get(), tex);

		scene.Add(std::move(go));
	}


	
	{
		auto fpsObj = std::make_unique<dae::GameObject>();

		auto tr =
			fpsObj->AddComponent<dae::TransformComponent>(fpsObj.get());
		tr->SetLocalPosition(20, 20);

		fpsObj->AddComponent<dae::TextComponent>(fpsObj.get(), "FPS", font);
		fpsObj->AddComponent<dae::FPSComponent>(fpsObj.get());

		scene.Add(std::move(fpsObj));
	}


	// Parent object
	{

		auto position  = std::make_unique<dae::GameObject>();
		auto positionTr = position->AddComponent<dae::TransformComponent>(position.get());
		positionTr->SetLocalPosition(512.f, 288.f);
		

		auto parent = std::make_unique<dae::GameObject>();
		auto parentTr = parent->AddComponent<dae::TransformComponent>(parent.get());
		parentTr->SetLocalPosition(0, 0);
		parentTr->SetLocalScale(0.05f, 0.05f);
		parent->SetParent(position.get());

		parent->AddComponent<dae::RenderComponent>(parent.get(),
			dae::ResourceManager::GetInstance().LoadTexture("pacman.png"));

		parent->AddComponent<dae::CircularMovementComponent>(parent.get(),50.f, 5.f);

		// Child object
		auto child = std::make_unique<dae::GameObject>();
		auto childTr = child->AddComponent<dae::TransformComponent>(child.get());
		childTr->SetLocalPosition(150.f, 0.f);

		child->AddComponent<dae::RenderComponent>(child.get(),
			dae::ResourceManager::GetInstance().LoadTexture("Pacman.png"));

		child->AddComponent<dae::CircularMovementComponent>(child.get(),50.f, -10.f);

		// Attach child
		child->SetParent(parent.get());

		// Add to scene
		scene.Add(std::move(position));
		scene.Add(std::move(parent));
		scene.Add(std::move(child));
	}

}

int main(int, char*[]) {
#if __EMSCRIPTEN__
	fs::path data_location = "";
#else
	fs::path data_location = "./Data/";
	if(!fs::exists(data_location))
		data_location = "../Data/";
#endif
	dae::Minigin engine(data_location);
	engine.Run(load);
    return 0;
}
