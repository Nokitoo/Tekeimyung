/**
* @Author   Guillaume Labey
*/

#include <Engine/Window/GameWindow.hpp>
#include <Engine/Utils/LogDebugWindow.hpp>
#include <Engine/Sound/SoundManager.hpp>

#include <Engine/Systems/RenderingSystem.hpp>
#include <Engine/Systems/MenuSystem.hpp>
#include <Engine/LevelEntitiesDebugWindow.hpp>
#include <Engine/EntitiesTemplateDebugWindow.hpp>
#include <Engine/SoundEditorWindow.hpp>
#include <Engine/EntityFactory.hpp>
#include <Engine/EditorMenuDebugWindow.hpp>
#include <Engine/EditorState.hpp>


EditorState::~EditorState() {}

void    EditorState::onEnter()
{
     EntityFactory::bindEntityManager(_world.getEntityManager());
}

bool    EditorState::init()
{
    _world.addSystem<ParticleSystem>();
    _world.addSystem<RenderingSystem>(&_camera, _world.getSystem<ParticleSystem>()->getEmitters());

    EntityManager* em = _world.getEntityManager();
    addDebugWindow<EditorMenuDebugWindow>(em, glm::vec2(0, 0), glm::vec2(0, 0));
    addDebugWindow<LevelEntitiesDebugWindow>(em, glm::vec2(0, 80), glm::vec2(600, 350));
    addDebugWindow<EntitiesTemplateDebugWindow>(em, glm::vec2(600, 80), glm::vec2(300, 200));
    addDebugWindow<SoundEditorWindow>(glm::vec2(1200, 80), glm::vec2(450, 450));
    addDebugWindow<LogDebugWindow>(Logger::getInstance(), glm::vec2(0, 430), glm::vec2(300, 200));

    initCamera();

    SoundManager::getInstance()->setVolume(0.3f);
    return (true);
}

bool    EditorState::update(float elapsedTime)
{
    return (GameState::update(elapsedTime));
}

void    EditorState::initCamera()
{
    _camera.translate(glm::vec3(350.0f, 250.0f, 300.0f));
    _camera.setDir(glm::vec3(-30.0f));

    // Set camera screen
    float size = 500.0f;
    Camera::sScreen screen;
    screen.right = size * _camera.getAspect();
    screen.left = -screen.right;
    screen.top = size;
    screen.bottom = -screen.top;
    _camera.setScreen(screen);
    _camera.setProjType(Camera::eProj::ORTHOGRAPHIC_3D);
    _camera.setZoom(0.5f);
}
