cmake_minimum_required(VERSION 3.26)
project(Programming4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(EMSCRIPTEN)
  message("Using Emscripten!")
endif()

include(FetchContent)

# =========================
# Dependencies
# =========================

# GLM
find_package(glm CONFIG QUIET)
if(NOT glm_FOUND)
  FetchContent_Declare(
    glm
    URL https://github.com/g-truc/glm/releases/download/1.0.3/glm-1.0.3.zip
    DOWNLOAD_NO_PROGRESS ON
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/downloads
  )
  FetchContent_MakeAvailable(glm)
endif()

# SDL3
find_package(SDL3 3.4 CONFIG QUIET)
if(NOT SDL3_FOUND)
  if(WIN32)
    FetchContent_Declare(
      SDL3
      URL https://www.libsdl.org/release/SDL3-devel-3.4.0-VC.zip
      DOWNLOAD_NO_PROGRESS ON
      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/downloads
    )
    FetchContent_MakeAvailable(SDL3)
    list(PREPEND CMAKE_PREFIX_PATH "${sdl3_SOURCE_DIR}")
    find_package(SDL3 CONFIG REQUIRED)
  else()
    FetchContent_Declare(
      SDL3
      GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
      GIT_TAG release-3.4.0
      GIT_SHALLOW TRUE
      GIT_PROGRESS TRUE
    )
    FetchContent_MakeAvailable(SDL3)
  endif()
endif()

# SDL3_ttf
find_package(SDL3_ttf 3.2.2 CONFIG QUIET)
if(NOT SDL3_ttf_FOUND)
  if(WIN32)
    FetchContent_Declare(
      SDL3_ttf
      URL https://www.libsdl.org/projects/SDL_ttf/release/SDL3_ttf-devel-3.2.2-VC.zip
      DOWNLOAD_NO_PROGRESS ON
      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/downloads
    )
    FetchContent_MakeAvailable(SDL3_ttf)
    list(PREPEND CMAKE_PREFIX_PATH "${sdl3_ttf_SOURCE_DIR}")
    find_package(SDL3_ttf CONFIG REQUIRED)
  else()
    FetchContent_Declare(
      SDL3_ttf
      GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
      GIT_TAG release-3.2.2
      GIT_SHALLOW TRUE
      GIT_PROGRESS TRUE
    )

    set(SDL3TTF_INSTALL OFF)

    if(EMSCRIPTEN)
      list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/emscripten")
      set(SDL3TTF_USE_SYSTEM_FREETYPE ON)
      set(SDL3TTF_VENDORED OFF)
      set(SDLTTF_PLUTOSVG OFF)
      add_compile_options(-sUSE_FREETYPE=1 -sUSE_HARFBUZZ=1)
      add_link_options(-sUSE_FREETYPE=1 -sUSE_HARFBUZZ=1)
    endif()

    FetchContent_MakeAvailable(SDL3_ttf)
  endif()
endif()

if(WIN32)
  find_package(VLD CONFIG)
endif()


# =========================
# ImGui
# =========================

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.9b
  GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer3.cpp
 )

target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC
  SDL3::SDL3
)
# =========================
# SDL3_mixer
# =========================

find_package(SDL3_mixer CONFIG QUIET)

if(NOT SDL3_mixer_FOUND)
  if(WIN32)
    FetchContent_Declare(
      SDL3_mixer
      URL https://www.libsdl.org/projects/SDL_mixer/release/SDL3_mixer-devel-3.2.0-VC.zip
      DOWNLOAD_NO_PROGRESS ON
      DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/downloads
    )

    FetchContent_MakeAvailable(SDL3_mixer)

    file(GLOB SDL3_MIXER_CONFIG_DIRS
      "${sdl3_mixer_SOURCE_DIR}/cmake"
      "${sdl3_mixer_SOURCE_DIR}/*/cmake"
    )

    find_package(SDL3_mixer CONFIG REQUIRED
      PATHS ${SDL3_MIXER_CONFIG_DIRS}
      NO_DEFAULT_PATH
    )

  else()
    FetchContent_Declare(
      SDL3_mixer
      GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
      GIT_TAG release-3.2.0
      GIT_SHALLOW TRUE
      GIT_PROGRESS TRUE
    )

    FetchContent_MakeAvailable(SDL3_mixer)
  endif()
endif()
# Steam
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/steam")
find_package(Steamworks QUIET)

# =========================
# Engine static library
# =========================

add_library(MiniginEngine STATIC
  Minigin/Font.cpp
  Minigin/GameObject.cpp
  Minigin/InputManager.cpp
  Minigin/Minigin.cpp
  Minigin/Renderer.cpp
  Minigin/ResourceManager.cpp
  Minigin/Scene.cpp
  Minigin/SceneManager.cpp
  Minigin/Texture2D.cpp
  Minigin/Transform.cpp
  Minigin/TransformComponent.cpp
  Minigin/RenderComponent.cpp
  Minigin/TextComponent.cpp
  Minigin/CircularMovementComponent.cpp
  Minigin/CacheTrashWindow.cpp
  Minigin/Controller.cpp

 
  Minigin/SDLSoundSystem.cpp
  Minigin/LoggingSoundSystem.cpp
 )

target_include_directories(MiniginEngine PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/Minigin
  ${VLD_INCLUDE_DIR}
)

target_compile_features(MiniginEngine PUBLIC cxx_std_20)

target_link_libraries(MiniginEngine PUBLIC
  SDL3::SDL3
  SDL3_ttf::SDL3_ttf
  SDL3_mixer::SDL3_mixer
  glm::glm
  imgui
  ${VLD_LIBRARIES}
)

if(WIN32 AND NOT EMSCRIPTEN)
  target_link_libraries(MiniginEngine PUBLIC xinput)
endif()

if(MSVC)
  target_compile_options(MiniginEngine PRIVATE /W4 /WX)
else()
  target_compile_options(MiniginEngine PRIVATE -Wall -Wextra -Werror)
endif()

# =========================
# Game executable
# =========================

if(EMSCRIPTEN)
  set(GAME_TARGET minigin_web)
  set(GAME_TARGET_PARAM)
else()
  set(GAME_TARGET minigin)
  set(GAME_TARGET_PARAM WIN32)
endif()

add_executable(${GAME_TARGET} ${GAME_TARGET_PARAM}
  DiggerGame/Main.cpp
  DiggerGame/MoveCommand.cpp
  DiggerGame/GameActorComponent.cpp
  DiggerGame/LoseLifeCommand.cpp
  DiggerGame/AddScoreCommand.cpp
  DiggerGame/LivesDisplayComponent.cpp
  DiggerGame/ScoreDisplayComponent.cpp
  DiggerGame/SteamAchievementComponent.cpp
  DiggerGame/GameManagerComponent.cpp
  DiggerGame/LevelLoader.cpp
  DiggerGame/GridPositionComponent.cpp
  DiggerGame/DiamondComponent.cpp
  DiggerGame/GridMoveCommand.cpp
  DiggerGame/SkipLevelCommand.cpp
  DiggerGame/ToggleMuteCommand.cpp
  DiggerGame/EnemyComponent.cpp
  DiggerGame/NobbinState.cpp
  DiggerGame/HobbinState.cpp
  DiggerGame/EnemyDeadState.cpp
)

target_include_directories(${GAME_TARGET} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/DiggerGame
)

target_link_libraries(${GAME_TARGET} PRIVATE
  MiniginEngine
)

if(Steamworks_FOUND)
  target_link_libraries(${GAME_TARGET} PRIVATE Steamworks::Steamworks)
  target_compile_definitions(${GAME_TARGET} PRIVATE USE_STEAMWORKS)
endif()

if(MSVC)
  target_compile_options(${GAME_TARGET} PRIVATE /W4 /WX)
else()
  target_compile_options(${GAME_TARGET} PRIVATE -Wall -Wextra -Werror)
endif()

# =========================
# Platform-specific setup
# =========================

if(EMSCRIPTEN)
  set(EMSCRIPTEN_COMPILE_FLAGS "-g")
  set(EMSCRIPTEN_LINK_FLAGS "-sUSE_FREETYPE=1 -sUSE_HARFBUZZ=1 -g -sALLOW_MEMORY_GROWTH --preload-file \"${CMAKE_CURRENT_SOURCE_DIR}/Data@/\"")

  set_target_properties(${GAME_TARGET} PROPERTIES
    COMPILE_FLAGS "${EMSCRIPTEN_COMPILE_FLAGS}"
    LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS}"
    SUFFIX ".html"
  )

  add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/web"
    "$<TARGET_FILE_DIR:${GAME_TARGET}>/"
  )

elseif(WIN32)

  set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${GAME_TARGET})

  set_target_properties(${GAME_TARGET} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${GAME_TARGET}>"
  )

  set(SDL_RUNTIME_DLLS
  SDL3::SDL3
  SDL3_ttf::SDL3_ttf
  SDL3_mixer::SDL3_mixer
)

  foreach(dllTarget IN LISTS SDL_RUNTIME_DLLS)
    add_custom_command(
      TARGET ${GAME_TARGET} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${dllTarget}>"
        "$<TARGET_FILE_DIR:${GAME_TARGET}>"
      VERBATIM
    )
  endforeach()

  if(Steamworks_FOUND)
    add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy
      "${Steamworks_RUNTIME_DLL}"
      "$<TARGET_FILE_DIR:${GAME_TARGET}>"
    )

    add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E echo "480" > "$<TARGET_FILE_DIR:${GAME_TARGET}>/steam_appid.txt"
      COMMENT "Generating steam_appid.txt"
    )
  endif()

  add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/Data"
    "$<TARGET_FILE_DIR:${GAME_TARGET}>/Data"
  )

endif()