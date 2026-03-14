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
#include "LivesDisplayObserver.h"
#include "LoseLifeCommand.h"
#include "ScoreDisplayObserver.h"
#include "AddScoreCommand.h"
#include "SteamAchievementObserver.h"

#include <filesystem>
namespace fs = std::filesystem;


static std::unique_ptr<dae::LivesDisplayObserver> g_P1LivesObserver;
static std::unique_ptr<dae::ScoreDisplayObserver> g_P1ScoreObserver;
static std::unique_ptr<dae::LivesDisplayObserver> g_P2LivesObserver;
static std::unique_ptr<dae::ScoreDisplayObserver> g_P2ScoreObserver;
static std::unique_ptr<dae::SteamAchievementObserver> g_SteamObserver;


static void load()
{


	
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
			dae::ResourceManager::GetInstance().LoadTexture("pacman.png"));


		
		auto ms_pack = std::make_unique<dae::GameObject>();
		auto ms_packTr = ms_pack->AddComponent<dae::TransformComponent>(ms_pack.get());
		ms_packTr->SetLocalPosition(250.f, 250.f);
		ms_packTr->SetLocalScale(0.05f, 0.05f);

		ms_pack->AddComponent<dae::RenderComponent>(ms_pack.get(),
			dae::ResourceManager::GetInstance().LoadTexture("Pacman.png"));


		auto p2 = ms_pack.get();

		auto* p2Actor = ms_pack->AddComponent<dae::GameActorComponent>(p2, 3, 0, 2);
		
		// Add to scene
		scene.Add(std::move(mr_packman));
		scene.Add(std::move(ms_pack));


		dae::TextComponent* p1LivesText{};
		dae::TextComponent* p1ScoreText{};
		dae::TextComponent* p2LivesText{};
		dae::TextComponent* p2ScoreText{};
		

		{
			auto ui = std::make_unique<dae::GameObject>();
			auto* uiObj = ui.get();

			ui->AddComponent<dae::TransformComponent>(uiObj)->SetLocalPosition(720.f, 50.f);
			p1LivesText = ui->AddComponent<dae::TextComponent>(uiObj, "Lives P1: 3", font);

			scene.Add(std::move(ui));
		}

		{
			auto ui = std::make_unique<dae::GameObject>();
			auto* uiObj = ui.get();

			ui->AddComponent<dae::TransformComponent>(uiObj)->SetLocalPosition(20.f, 70.f);
			ui->AddComponent<dae::TextComponent>(uiObj, "Player 1 - WASD to Move, Q to lose Life, E to Add score", font2);

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
			p1ScoreText = ui->AddComponent<dae::TextComponent>(uiObj, "Score P1: 0", font);

			scene.Add(std::move(ui));
		}

		{
			auto ui = std::make_unique<dae::GameObject>();
			auto* uiObj = ui.get();

			ui->AddComponent<dae::TransformComponent>(uiObj)->SetLocalPosition(720.f, 120.f);
			p2LivesText = ui->AddComponent<dae::TextComponent>(uiObj, "Lives P2: 3", font);

			scene.Add(std::move(ui));
		}

		{
			auto ui = std::make_unique<dae::GameObject>();
			auto* uiObj = ui.get();

			ui->AddComponent<dae::TransformComponent>(uiObj)->SetLocalPosition(720.f, 150.f);
			p2ScoreText = ui->AddComponent<dae::TextComponent>(uiObj, "Score P2: 0", font);

			scene.Add(std::move(ui));
		}

		// Observers
		g_P1LivesObserver = std::make_unique<dae::LivesDisplayObserver>(p1Actor, p1LivesText, "Lives P1: ");
		g_P1LivesObserver->StartListening();

		g_P1ScoreObserver = std::make_unique<dae::ScoreDisplayObserver>(p1Actor, p1ScoreText, "Score P1: ");
		g_P1ScoreObserver->StartListening();

		g_P2LivesObserver = std::make_unique<dae::LivesDisplayObserver>(p2Actor, p2LivesText, "Lives P2: ");
		g_P2LivesObserver->StartListening();

		g_P2ScoreObserver = std::make_unique<dae::ScoreDisplayObserver>(p2Actor, p2ScoreText, "Score P2: ");
		g_P2ScoreObserver->StartListening();

		g_SteamObserver = std::make_unique<dae::SteamAchievementObserver>(p1Actor);
		g_SteamObserver->StartListening();


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
			std::make_unique<dae::LoseLifeCommand>(p1Actor));
		input.BindKeyboardCommand(SDL_SCANCODE_E, dae::KeyState::Down,
			std::make_unique<dae::AddScoreCommand>(p1Actor, 100));

		input.BindControllerCommand(0, dae::ControllerButton::ButtonA, dae::KeyState::Down,
			std::make_unique<dae::LoseLifeCommand>(p2Actor));
		input.BindControllerCommand(0, dae::ControllerButton::ButtonB, dae::KeyState::Down,
			std::make_unique<dae::AddScoreCommand>(p2Actor, 100));

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
