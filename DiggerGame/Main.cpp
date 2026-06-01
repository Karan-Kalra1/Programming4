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
#include "GameManagerComponent.h"
#include "SkipLevelCommand.h"
#include "ToggleMuteCommand.h"
#include "GridMoveCommand.h"
#include "ShootFireballCommand.h"
#include "HighScoreConfirmCommand.h"
#include "HighScoreCursorCommand.h"
#include "HighScoreLetterCommand.h"
#include "MenuConfirmCommand.h"
#include "MenuNavigateCommand.h"

#include <filesystem>
namespace fs = std::filesystem;





static void load()
{
	auto& scene = dae::SceneManager::GetInstance().CreateScene();

	auto managerObject = std::make_unique<dae::GameObject>();
	auto* managerGo = managerObject.get();

	auto* manager =
		managerObject->AddComponent<digger::GameManagerComponent>(managerGo, &scene);

	manager->ShowStartMenu();

	auto& input = dae::InputManager::GetInstance();

	input.BindKeyboardCommand(SDL_SCANCODE_F1, dae::KeyState::Down,
		std::make_unique<digger::SkipLevelCommand>(manager));

	input.BindKeyboardCommand(SDL_SCANCODE_F2, dae::KeyState::Down,
		std::make_unique<digger::ToggleMuteCommand>(manager));

	input.BindKeyboardCommand(SDL_SCANCODE_W, dae::KeyState::Down,
		std::make_unique<digger::GridMoveCommand>( manager, glm::ivec2{ 0, -1 },true));
	input.BindKeyboardCommand(SDL_SCANCODE_W, dae::KeyState::Up,
		std::make_unique<digger::GridMoveCommand>(manager, glm::ivec2{ 0, -1 },false));

	input.BindKeyboardCommand(SDL_SCANCODE_S, dae::KeyState::Down,
		std::make_unique<digger::GridMoveCommand>( manager, glm::ivec2{ 0, 1 },true));
	input.BindKeyboardCommand(SDL_SCANCODE_S, dae::KeyState::Up,
		std::make_unique<digger::GridMoveCommand>(manager, glm::ivec2{ 0, 1 }, false));

	input.BindKeyboardCommand(SDL_SCANCODE_A, dae::KeyState::Down,
		std::make_unique<digger::GridMoveCommand>( manager, glm::ivec2{ -1, 0 },true));
	input.BindKeyboardCommand(SDL_SCANCODE_A, dae::KeyState::Up,
		std::make_unique<digger::GridMoveCommand>(manager, glm::ivec2{ -1, 0 }, false));

	input.BindKeyboardCommand(SDL_SCANCODE_D, dae::KeyState::Down,
		std::make_unique<digger::GridMoveCommand>( manager, glm::ivec2{ 1, 0 },true));
	input.BindKeyboardCommand(SDL_SCANCODE_D, dae::KeyState::Up,
		std::make_unique<digger::GridMoveCommand>(manager, glm::ivec2{ 1, 0 }, false));
	input.BindKeyboardCommand(SDL_SCANCODE_SPACE, dae::KeyState::Down,
		std::make_unique<digger::ShootFireballCommand>(manager));
	input.BindControllerCommand(0, dae::ControllerButton::ButtonA, dae::KeyState::Down,
		std::make_unique<digger::ShootFireballCommand>(manager));


	input.BindKeyboardCommand(SDL_SCANCODE_UP, dae::KeyState::Down,
		std::make_unique<digger::HighScoreLetterCommand>(manager, 1));

	input.BindKeyboardCommand(SDL_SCANCODE_DOWN, dae::KeyState::Down,
		std::make_unique<digger::HighScoreLetterCommand>(manager, -1));

	input.BindKeyboardCommand(SDL_SCANCODE_LEFT, dae::KeyState::Down,
		std::make_unique<digger::HighScoreCursorCommand>(manager, -1));

	input.BindKeyboardCommand(SDL_SCANCODE_RIGHT, dae::KeyState::Down,
		std::make_unique<digger::HighScoreCursorCommand>(manager, 1));

	input.BindKeyboardCommand(SDL_SCANCODE_RETURN, dae::KeyState::Down,
		std::make_unique<digger::HighScoreConfirmCommand>(manager));


	input.BindControllerCommand(0, dae::ControllerButton::DPadUp, dae::KeyState::Down,
		std::make_unique<digger::HighScoreLetterCommand>(manager, 1));

	input.BindControllerCommand(0, dae::ControllerButton::DPadDown, dae::KeyState::Down,
		std::make_unique<digger::HighScoreLetterCommand>(manager, -1));

	input.BindControllerCommand(0, dae::ControllerButton::DPadLeft, dae::KeyState::Down,
		std::make_unique<digger::HighScoreCursorCommand>(manager, -1));

	input.BindControllerCommand(0, dae::ControllerButton::DPadRight, dae::KeyState::Down,
		std::make_unique<digger::HighScoreCursorCommand>(manager, 1));

	input.BindControllerCommand(0, dae::ControllerButton::ButtonA, dae::KeyState::Down,
		std::make_unique<digger::HighScoreConfirmCommand>(manager));


	input.BindKeyboardCommand(SDL_SCANCODE_UP, dae::KeyState::Down,
		std::make_unique<digger::MenuNavigateCommand>(manager, -1));

	input.BindKeyboardCommand(SDL_SCANCODE_DOWN, dae::KeyState::Down,
		std::make_unique<digger::MenuNavigateCommand>(manager, 1));

	input.BindKeyboardCommand(SDL_SCANCODE_RETURN, dae::KeyState::Down,
		std::make_unique<digger::MenuConfirmCommand>(manager));


	input.BindControllerCommand(0, dae::ControllerButton::DPadUp, dae::KeyState::Down,
		std::make_unique<digger::MenuNavigateCommand>(manager, -1));

	input.BindControllerCommand(0, dae::ControllerButton::DPadDown, dae::KeyState::Down,
		std::make_unique<digger::MenuNavigateCommand>(manager, 1));

	input.BindControllerCommand(0, dae::ControllerButton::ButtonA, dae::KeyState::Down,
		std::make_unique<digger::MenuConfirmCommand>(manager));

	scene.Add(std::move(managerObject));
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
