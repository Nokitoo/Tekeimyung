/*
** @Author: Simon AMBROISE
*/

#include <Engine/Systems/MouseSystem.hpp>

#include <Engine/Components.hh>
#include <Engine/Physics/Collisions.hpp>
#include <Engine/Window/GameWindow.hpp>
#include <Engine/Graphics/Renderer.hpp>

MouseSystem::MouseSystem()
{
    _monitoringKey = MonitoringDebugWindow::getInstance()->registerSystem(MOUSE_SYSTEM_NAME);
}

void MouseSystem::update(EntityManager &em, float elapsedTime)
{
    Timer   timer;
    uint32_t nbEntities = 0;

    this->hoverEntity(em);

    MonitoringDebugWindow::getInstance()->updateSystem(_monitoringKey, timer.getElapsedTime(), nbEntities);
}

void MouseSystem::hoverEntity(EntityManager& em)
{
    auto gameWindow = GameWindow::getInstance();
    Mouse& mouse = gameWindow->getMouse();
    Cursor& cursor = mouse.getCursor();

    glm::vec3 nearScreen(cursor.getX(), gameWindow->getBufferHeight() - cursor.getY(), 0.0f);
    glm::vec3 farScreen(cursor.getX(), gameWindow->getBufferHeight() - cursor.getY(), 1.0f);

    Camera* camera = Renderer::getInstance()->getCurrentCamera();

    if (!camera)
        return;

    // Unproject 2D points to get 3D points
    // Get 3D point on near plane
    glm::vec3 nearPoint = glm::unProject(nearScreen, camera->getView(), camera->getProj(), gameWindow->getViewport());
    // Get 3D point on far plane
    glm::vec3 farPoint = glm::unProject(farScreen, camera->getView(), camera->getProj(), gameWindow->getViewport());

    // Calculate vector from near plane to far plane
    glm::vec3 rayDir = farPoint - nearPoint;

    Entity* selectedEntity = nullptr;
    float selectedEntityDist = 0.0f;

    for (auto it : em.getEntities())
    {
        Entity* entity = it.second;

        sRenderComponent* render = entity->getComponent<sRenderComponent>();
        sTransformComponent* transform = entity->getComponent<sTransformComponent>();

        // We can't select entity that is not displayed or has model not initialized
        if (!render || !render->_model)
            continue;

        // Model box collider position
        glm::vec3 boxMin = glm::vec3(render->_model->getMin().x, render->_model->getMin().y, render->_model->getMin().z);
        glm::vec3 boxMax = glm::vec3(render->_model->getMax().x, render->_model->getMax().y, render->_model->getMax().z);

        boxMin *= transform->scale;
        boxMax *= transform->scale;

        // Convert box collider to world position
        boxMin += transform->pos;
        boxMax += transform->pos;

        float distance = Collisions::rayVSAABB(nearPoint, rayDir, boxMin, boxMax);
        if (distance != 0 && (selectedEntity == nullptr || distance <= selectedEntityDist))
        {
            selectedEntityDist = distance;
            selectedEntity = entity;
        }
    }

    if (selectedEntity != nullptr)
    {
        if (this->previousEntity != nullptr && this->previousEntity->id != selectedEntity->id)
        {
            if (this->previousEntity->getComponent<sScriptComponent>() != nullptr)
                for (auto&& script : this->previousEntity->getComponent<sScriptComponent>()->scriptInstances)
                    script->OnHoverExit();

            if (selectedEntity->getComponent<sScriptComponent>() != nullptr)
                for (auto&& script : selectedEntity->getComponent<sScriptComponent>()->scriptInstances)
                    script->OnHoverEnter();

            this->previousEntity = selectedEntity;
        }
        else if (this->previousEntity == nullptr)
        {
            if (selectedEntity->getComponent<sScriptComponent>() != nullptr)
                for (auto&& script : selectedEntity->getComponent<sScriptComponent>()->scriptInstances)
                    script->OnHoverEnter();

            this->previousEntity = selectedEntity;
        }
    }
    else
    {
        if (this->previousEntity != nullptr)
        {
            if (this->previousEntity->getComponent<sScriptComponent>() != nullptr)
                for (auto&& script : this->previousEntity->getComponent<sScriptComponent>()->scriptInstances)
                    script->OnHoverExit();

            this->previousEntity = nullptr;
        }
    }
}
