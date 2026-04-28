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
#include "InputManager.h"
#include "MoveCommand.h"
#include "Controller.h"
#include "GameActorComponent.h"
#include "LivesDisplayComponent.h"
#include "LoseLifeCommand.h"
#include "ScoreDisplayComponent.h"
#include "AddScoreCommand.h"
#include "EventBus.h"
#include "SteamAchievementComponent.h"
#include "ServiceLocator.h"
#include "SDLSoundSystem.h"
#include "LoggingSoundSystem.h"
#include "PlaySoundCommand.h"

#include <filesystem>
namespace fs = std::filesystem;





static void load()
{

	enum GameSound : dae::SoundId
	{
		DiamondPickUp = 1
	};

	dae::ServiceLocator::GetSoundSystem().RegisterSound(
		GameSound::DiamondPickUp,
		"Data/sounds/PickUpDiamond.wav");
	
	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	
	auto font =
		dae::ResourceManager::GetInstance()
		.LoadFont("Lingua.otf", 36);

	auto font2 =
		dae::ResourceManager::GetInstance()
		.LoadFont("Lingua.otf", 16);
	
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


	{

		

		auto mr_packman = std::make_unique<dae::GameObject>();
		auto mr_packmanTr = mr_packman->AddComponent<dae::TransformComponent>(mr_packman.get());
		mr_packmanTr->SetLocalPosition(350, 350);
		mr_packmanTr->SetLocalScale(0.05f, 0.05f);
		 
		auto p1 = mr_packman.get();

		auto* p1Actor = mr_packman->AddComponent<dae::GameActorComponent>(p1, 3, 0, 1);

		mr_packman->AddComponent<dae::RenderComponent>(mr_packman.get(),
			dae::ResourceManager::GetInstance().LoadTexture("Pacman.png"));

		dae::EventBus::GetInstance().GetSubject().AddObserver(p1Actor);


		
		auto ms_pack = std::make_unique<dae::GameObject>();
		auto ms_packTr = ms_pack->AddComponent<dae::TransformComponent>(ms_pack.get());
		ms_packTr->SetLocalPosition(250.f, 250.f);
		ms_packTr->SetLocalScale(0.05f, 0.05f);

		ms_pack->AddComponent<dae::RenderComponent>(ms_pack.get(),
			dae::ResourceManager::GetInstance().LoadTexture("Pacman.png"));


		auto p2 = ms_pack.get();

		auto* p2Actor = ms_pack->AddComponent<dae::GameActorComponent>(p2, 3, 0, 2);

		dae::EventBus::GetInstance().GetSubject().AddObserver(p2Actor);
		
		// Add to scene
		scene.Add(std::move(mr_packman));
		scene.Add(std::move(ms_pack));
	

		{
			auto ui = std::make_unique<dae::GameObject>();
			auto* uiObj = ui.get();

			ui->AddComponent<dae::TransformComponent>(uiObj)->SetLocalPosition(720.f, 50.f);
			ui->AddComponent<dae::TextComponent>(uiObj, "Lives P1: 3", font);
			ui->AddComponent<dae::LivesDisplayComponent>(uiObj, p1Actor, "Lives P1: ");
			scene.Add(std::move(ui));
		}

		{
			auto ui = std::make_unique<dae::GameObject>();
			auto* uiObj = ui.get();

			ui->AddComponent<dae::TransformComponent>(uiObj)->SetLocalPosition(20.f, 70.f);
			ui->AddComponent<dae::TextComponent>(uiObj, "Player 1 - WASD to Move, Q to lose Life, E to Add score, P to play Sound", font2);

			scene.Add(std::move(ui));
		}

		{
			auto ui = std::make_unique<dae::GameObject>();
			auto* uiObj = ui.get();

			ui->AddComponent<dae::TransformComponent>(uiObj)->SetLocalPosition(20.f, 110.f);
			ui->AddComponent<dae::TextComponent>(uiObj, "Player 2 - D-Pad to Move, A to lose Life, B to Add score", font2);

			scene.Add(std::move(ui));
		}

		{
			auto ui = std::make_unique<dae::GameObject>();
			auto* uiObj = ui.get();

			ui->AddComponent<dae::TransformComponent>(uiObj)->SetLocalPosition(720.f, 80.f);
			ui->AddComponent<dae::TextComponent>(uiObj, "Score P1: 0", font);
			ui->AddComponent<dae::ScoreDisplayComponent>(uiObj, p1Actor, "Score P1: ");
			scene.Add(std::move(ui));
		}

		{
			auto ui = std::make_unique<dae::GameObject>();
			auto* uiObj = ui.get();

			ui->AddComponent<dae::TransformComponent>(uiObj)->SetLocalPosition(720.f, 120.f);
			ui->AddComponent<dae::TextComponent>(uiObj, "Lives P2: 3", font);
			ui->AddComponent<dae::LivesDisplayComponent>(uiObj, p2Actor, "Lives P2: ");
			scene.Add(std::move(ui));
		}

		{
			auto ui = std::make_unique<dae::GameObject>();
			auto* uiObj = ui.get();

			ui->AddComponent<dae::TransformComponent>(uiObj)->SetLocalPosition(720.f, 150.f);
			ui->AddComponent<dae::TextComponent>(uiObj, "Score P2: 0", font);
			ui->AddComponent<dae::ScoreDisplayComponent>(uiObj, p2Actor, "Score P2: ");
			scene.Add(std::move(ui));
		}
	
		// Steam observer component on a dummy
		{
			auto steamGo = std::make_unique<dae::GameObject>();
			auto* steamObj = steamGo.get();
			steamGo->AddComponent<dae::SteamAchievementComponent>(steamObj, p1Actor);
			scene.Add(std::move(steamGo));
		}

		constexpr float speed1{ 100.f };
		constexpr float speed2{ 200.f };

		auto& input = dae::InputManager::GetInstance();


		input.BindKeyboardCommand(SDL_SCANCODE_W, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(p1, glm::vec2{ 0.f, -1.f }, speed1));
		input.BindKeyboardCommand(SDL_SCANCODE_S, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(p1, glm::vec2{ 0.f, 1.f }, speed1));
		input.BindKeyboardCommand(SDL_SCANCODE_A, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(p1, glm::vec2{ -1.f, 0.f }, speed1));
		input.BindKeyboardCommand(SDL_SCANCODE_D, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(p1, glm::vec2{ 1.f, 0.f }, speed1));

		input.BindControllerCommand(0, dae::ControllerButton::DPadUp, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(p2, glm::vec2{ 0.f, -1.f }, speed2));
		input.BindControllerCommand(0, dae::ControllerButton::DPadDown, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(p2, glm::vec2{ 0.f, 1.f }, speed2));
		input.BindControllerCommand(0, dae::ControllerButton::DPadLeft, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(p2, glm::vec2{ -1.f, 0.f }, speed2));
		input.BindControllerCommand(0, dae::ControllerButton::DPadRight, dae::KeyState::Pressed, std::make_unique<dae::MoveCommand>(p2, glm::vec2{ 1.f, 0.f }, speed2));


		input.BindKeyboardCommand(SDL_SCANCODE_Q, dae::KeyState::Down,
			std::make_unique<dae::LoseLifeCommand>(p1,1));
		input.BindKeyboardCommand(SDL_SCANCODE_E, dae::KeyState::Down,
			std::make_unique<dae::AddScoreCommand>(p1, 100));

		input.BindControllerCommand(0, dae::ControllerButton::ButtonA, dae::KeyState::Down,
			std::make_unique<dae::LoseLifeCommand>(p2,1));
		input.BindControllerCommand(0, dae::ControllerButton::ButtonB, dae::KeyState::Down,
			std::make_unique<dae::AddScoreCommand>(p2, 100));

		input.BindKeyboardCommand(SDL_SCANCODE_P, dae::KeyState::Down,
			std::make_unique<dae::PlaySoundCommand>(GameSound::DiamondPickUp, 1.0f));
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
#if _DEBUG
	dae::ServiceLocator::RegisterSoundSystem(
		std::make_unique<dae::LoggingSoundSystem>(
			std::make_unique<dae::SDLSoundSystem>()));
#else
	dae::ServiceLocator::RegisterSoundSystem(
		std::make_unique<dae::SDLSoundSystem>());
#endif
	engine.Run(load);
    return 0;
}
