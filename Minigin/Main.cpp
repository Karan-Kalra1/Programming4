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

		auto transform =
			go->AddComponent<dae::TransformComponent>(go.get());
		transform->SetPosition(0, 0);

		auto tex =
			dae::ResourceManager::GetInstance()
			.LoadTexture("background.png");

		go->AddComponent<dae::RenderComponent>(go.get(),tex);

		scene.Add(std::move(go));
	}

	
	{
		auto go = std::make_unique<dae::GameObject>();

		auto transform =
			go->AddComponent<dae::TransformComponent>(go.get());
		transform->SetPosition(358, 180);

		auto tex =
			dae::ResourceManager::GetInstance()
			.LoadTexture("logo.png");

		go->AddComponent<dae::RenderComponent>(go.get(), tex);

		scene.Add(std::move(go));
	}


	
	{
		auto fpsObj = std::make_unique<dae::GameObject>();

		auto tr =
			fpsObj->AddComponent<dae::TransformComponent>(fpsObj.get());
		tr->SetPosition(20, 20);

		fpsObj->AddComponent<dae::TextComponent>(fpsObj.get(), "FPS", font);
		fpsObj->AddComponent<dae::FPSComponent>(fpsObj.get());

		scene.Add(std::move(fpsObj));
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
