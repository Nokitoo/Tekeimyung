/**
* @Author   Guillaume Labey
*/

#pragma once

#include <memory>

#include <Engine/Window/GameWindow.hpp>
#include <Engine/Sound/SoundManager.hpp>
#include <Engine/Graphics/Renderer.hpp>
#include <Engine/Utils/Logger.hpp>

#include <Engine/Core/GameStateManager.hpp>

class Engine
{
public:
    Engine();
    ~Engine();

    bool                                    init();
    bool                                    run();
    bool                                    stop();
    GameStateManager&                       getGameStateManager();

private:
    GameStateManager                        _gameStateManager;
    std::shared_ptr<GameWindow>             _window;
    std::shared_ptr<SoundManager>           _soundManager;
    std::shared_ptr<Renderer>               _renderer;
    std::shared_ptr<Logger>                 _logger;
};
