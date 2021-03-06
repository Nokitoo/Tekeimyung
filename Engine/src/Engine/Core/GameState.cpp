/**
* @Author   Guillaume Labey
*/

#include <Engine/Core/GameStateManager.hpp>
#include <Engine/Systems/RenderingSystem.hpp>
#include <Engine/Utils/LevelLoader.hpp>
#include <Engine/Debug/Debug.hpp>
#include <Engine/Utils/Exception.hpp>
#include <Engine/Debug/MonitoringDebugWindow.hpp>
#include <Engine/Window/GameWindow.hpp>
#include <Engine/EditorState.hpp>
#include <Engine/EntityFactory.hpp>
#include <Engine/Systems/UISystem.hpp>

#include <Engine/Core/GameState.hpp>

GameState::GameState(GameStateManager* gameStateManager, uint32_t id, const std::string& levelFile):
                    _gameStateManager(gameStateManager), _id(id), _levelFile(levelFile) {}

GameState::~GameState() {}

bool    GameState::initSystems()
{
    for (auto& system: _world.getSystems())
    {
        if (!system->init())
        {
            return (false);
        }
    }

    return (true);
}

bool    GameState::update(float elapsedTime)
{
    try
    {
        // Update GameState systems
        for (auto&& system: _world.getSystems())
        {
            Timer timer;
            system->update(*_world.getEntityManager(), elapsedTime * _timeSpeed);
            MonitoringDebugWindow::getInstance()->updateSystem(system->getId(), timer.getElapsedTime(), system->getEntitiesNb(), system->getName());
        }
    }
    catch(const Exception &e)
    {
        std::cerr << e.what() << std::endl;
        return (false);
    }

    // Destroy entities in destroy queue
    _world.getEntityManager()->destroyEntities();

    return (true);
}

uint32_t    GameState::getId() const
{
    return (_id);
}

float    GameState::getTimeSpeed() const
{
    return (_timeSpeed);
}

World&  GameState::getWorld()
{
    return (_world);
}

void    GameState::setLevelFile(const std::string& levelFile)
{
    _levelFile = levelFile;
}

void    GameState::setTimeSpeed(float timeSpeed)
{
    _timeSpeed = timeSpeed;
}

void    GameState::cloneEntityManager(EntityManager* em)
{
    EntityFactory::copyEntityManager(_world.getEntityManager(), em);
}

void    GameState::renderPreviousStates(const std::vector<uint32_t>& filterIds)
{
    uint32_t gameStateIdx = 0;
    uint32_t statesNb = (uint32_t)_gameStateManager->getStates().size();

    // Find GameState index in states vector
    for (gameStateIdx; gameStateIdx < statesNb; ++gameStateIdx)
    {
        if (_gameStateManager->getStates()[gameStateIdx].get() == this)
        {
            break;
        }
    }
    ASSERT(gameStateIdx != statesNb, "The GameState should be in states vector");

    if (gameStateIdx == 0)
    {
        return;
    }

    for (uint32_t i = 0; i < gameStateIdx; ++i)
    {
        auto state = _gameStateManager->getStates()[i];
        World& stateWorld = state->getWorld();

        if (filterIds.size() > 0 &&
            std::find(filterIds.begin(), filterIds.end(), state->getId()) == filterIds.end())
            continue;

        #if defined(ENGINE_DEBUG)
            if (state->getId() == EditorState::identifier && gameStateIdx > 1)
                continue;
        #endif

        System* stateRenderSystem = stateWorld.getSystem<RenderingSystem>();
        System* stateUISystem = stateWorld.getSystem<UISystem>();

        if (stateRenderSystem)
        {
            stateRenderSystem->update(*stateWorld.getEntityManager(), 0);
        }
        if (stateUISystem)
        {
            stateUISystem->update(*stateWorld.getEntityManager(), 0);
        }
    }
}

void    GameState::bindEntityManager()
{
    EntityFactory::bindEntityManager(_world.getEntityManager());
}

void    GameState::loadLevel()
{
    // Load level
    if (_levelFile.size() > 0)
    {
        LOG_INFO("Loading level \"%s\"", _levelFile.c_str());
        LevelLoader::getInstance()->load(_levelFile, _world.getEntityManager());
    }
}

void    GameState::onWindowResize()
{
    // Update UI system
    {
        UISystem* uiSystem = _world.getSystem<UISystem>();
        if (uiSystem)
        {
            uiSystem->onWindowResize(*_world.getEntityManager());
        }
    }
}
