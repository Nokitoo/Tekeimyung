#include <imgui.h>

#include <Engine/Utils/Logger.hpp>
#include <Engine/Utils/Timer.hpp>

#include <Engine/Core/Engine.hpp>

Engine::Engine() {}

Engine::~Engine() {}

bool    Engine::init()
{
    _window = std::make_shared<GameWindow>(&_gameStateManager);
    if (!_window->initialize())
        return (false);

    _logger = Logger::getInstance();
    if (!_logger->initialize())
        return (false);

    _soundManager = SoundManager::getInstance();
    if (!_soundManager->initialize())
        return (false);

    _renderer = Renderer::getInstance();
    if (!_renderer->initialize())
        return (false);

    GameWindow::setInstance(_window);
    return (true);
}

bool    Engine::run()
{
    // FPS counter
    Timer&       timer = _window->getTimer();
    float       timePerFrame;

    timer.reset();
    timePerFrame = 1.0f / 60.0f;
    while (_window->isRunning())
    {
        // Run one frame each 16ms
        if (timer.getElapsedTime() >= timePerFrame)
        {
            float elapsedTime = timer.getElapsedTime();
            timer.reset();
            _window->getKeyboard().updateKeyboardState();
            _window->getMouse().updateMouseState();
            _window->pollEvents();

            _soundManager->update();

            if (!_gameStateManager.hasStates())
            {
                return (true);
            }

            auto &&currentState = _gameStateManager.getCurrentState();

            // Clear color buffer
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (currentState->update(elapsedTime) == false)
            {
                _gameStateManager.removeCurrentState();
            }


            // Display imgui windows
            ImGui::Render();

            // Display screen
            GameWindow::getInstance()->display();
        }
    }
    return (true);
}

bool    Engine::stop()
{
    _soundManager->shutdown();
    Logger::getInstance()->shutdown();
    return (true);
}

GameStateManager&   Engine::getGameStateManager()
{
    return (_gameStateManager);
}
