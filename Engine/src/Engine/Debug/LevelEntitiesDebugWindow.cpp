/**
* @Author   Guillaume Labey
*/

#include <imgui.h>

#include <Engine/Core/Components/IComponentFactory.hpp>
#include <Engine/Core/Components/NameComponent.hh>
#include <Engine/EntityFactory.hpp>
#include <Engine/Window/GameWindow.hpp>

#include <Engine/Debug/LevelEntitiesDebugWindow.hpp>

Entity::sHandle    LevelEntitiesDebugWindow::_selectedEntityHandler = 0;

LevelEntitiesDebugWindow::LevelEntitiesDebugWindow() : DebugWindow("Scene Hierarchy") {}

LevelEntitiesDebugWindow::LevelEntitiesDebugWindow(const glm::vec2& pos, const glm::vec2& size) :
    DebugWindow("Scene Hierarchy", pos, size) {}

LevelEntitiesDebugWindow::~LevelEntitiesDebugWindow() {}

void    LevelEntitiesDebugWindow::build(std::shared_ptr<GameState> gameState, float elapsedTime)
{
    if (!ImGui::Begin(_title.c_str(), &_displayed))
    {
        ImGui::End();
        return;
    }

    // Entities list
    static ImGuiTextFilter filter;

    EntityManager* em = gameState->getWorld().getEntityManager();

    filter.Draw();
    ImGui::BeginChild("Entities list", ImVec2(150, 0), true);
    for (Entity* entity: em->getEntities())
    {
        sNameComponent* name = entity->getComponent<sNameComponent>();

        ASSERT(name != nullptr, "The entity should have a name");
        if (filter.PassFilter(name->value.c_str()) == true)
        {
            ImGui::PushID(static_cast<uint32_t>(entity->handle));
            if (ImGui::Selectable(name->value.c_str(), _selectedEntityHandler == entity->handle))
            {
                _selectedEntityHandler = entity->handle;
            }
            ImGui::PopID();
        }
    }
    ImGui::EndChild();

    if (em->getEntities().size() == 0)
    {
        ImGui::End();
        return;
    }

    Entity* selectedEntity = em->getEntity(_selectedEntityHandler);

    // The entity has been deleted or none is selected
    if (!selectedEntity)
    {
        ImGui::End();
        return;
    }

    ImGui::End();
}

void    LevelEntitiesDebugWindow::displayEntityDebug(EntityManager* em, Entity* entity)
{
    ImGui::SameLine();
    ImGui::BeginGroup();
    sNameComponent* nameComp = entity->getComponent<sNameComponent>();
    ASSERT(nameComp != nullptr, "The entity should have a name");

    std::string entityName = nameComp->value;
    ImGui::PushItemWidth(200);
    if (ImGui::CollapsingHeader(entityName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Add component button
        ImGui::PushStyleColor(ImGuiCol_Button, ImColor(0.27f, 0.51f, 0.70f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor(0.39f, 0.58f, 0.92f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor(0.49f, 0.68f, 0.92f, 1.0f));

        // Delete the entity
        if (ImGui::Button("Delete"))
        {
            ImGui::EndGroup();
            em->destroyEntity(entity->handle);
            ImGui::PopStyleColor(3);
            return;
        }

        // Add component to entity
        if (ImGui::Button("Add component"))
        {
            ImGui::OpenPopup("components");
        }

        ImGui::SameLine();
        // Clone the template
        if (ImGui::Button("Create template"))
        {
            ImGui::OpenPopup("create template");
        }

        // Clone template: specify new type name
        if (ImGui::BeginPopup("create template"))
        {
            static char newTypeName[64] = "";

            ImGui::InputText("New type", newTypeName, 64);
            if (ImGui::Button("Create"))
            {
                createTemplate(entity, newTypeName);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Save entity template to json
        // And apply changes to all entities of the same type in the scene
        ImGui::SameLine();
        if (ImGui::Button("Apply changes to template"))
        {
            EntityFactory::saveEntityTemplate(entityName, entity);
        }

        ImGui::PopStyleColor(3);

        // Edit entity tag
        ImGui::Text("\n");
        const std::string& entityTag = entity->getTag();
        std::vector<char> entityTagVec(entityTag.cbegin(), entityTag.cend());
        entityTagVec.push_back(0);
        entityTagVec.resize(64);

        if (ImGui::InputText("Tag", entityTagVec.data(), entityTagVec.size()))
        {
            entity->setTag(entityTagVec.data());
        }

        // Display new component that can be added
        if (ImGui::BeginPopup("components"))
        {
            for (auto componentHash: IComponentFactory::getComponentsTypesHashs())
            {
                // Entity does not have this component, display it
                if (!entity->hasComponent(componentHash.first))
                {
                    // Component button pressed, add it to the entity
                    if (ImGui::Button(componentHash.second.c_str()))
                    {
                        // Get component factory to create a new one
                        IComponentFactory* newComponentFactory = IComponentFactory::getFactory(componentHash.second);
                        ASSERT(newComponentFactory != nullptr, "The component factory should exist");

                        // Send fake entity name and json to loadFromJson to have the component fields initialized with default values
                        sComponent* newComponent = newComponentFactory->loadFromJson("", {});
                        entity->addComponent(newComponent);
                    }
                }
            }
            ImGui::EndPopup();
        }

        // Display all components debug
        for (uint32_t i = 0; i < entity->getComponents().size(); ++i)
        {
            sComponent* component = entity->getComponents()[i];
            std::string componentName = IComponentFactory::getComponentNameWithHash(component->id);

            ASSERT(componentName.size() > 0, "The component name should exist");
            IComponentFactory*  compFactory = IComponentFactory::getFactory(componentName);
            sComponent*         savedComponent = nullptr;

            // Display component debug
            if (ImGui::CollapsingHeader(componentName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                // Remove component button
                ImGui::PushID(componentName.c_str());
                if (componentName != "sTransformComponent" &&
                    componentName != "sNameComponent" &&
                    ImGui::Button("Remove component"))
                {
                    entity->removeComponent(component);
                    --i;
                }

                // Component debug content
                else if (compFactory->updateEditor(entityName, &savedComponent, component, entity))
                {
                    ASSERT(component != nullptr, "component should be set in updateEditor");
                }
                ImGui::PopID();
            }
        }
    }
    ImGui::EndGroup();
}

Entity::sHandle    LevelEntitiesDebugWindow::getSelectedEntityHandler()
{
    return (_selectedEntityHandler);
}

void    LevelEntitiesDebugWindow::createTemplate(Entity* entity, const std::string& newTypeName)
{
    if (EntityFactory::entityTypeExists(newTypeName))
    {
        LOG_ERROR("Can't create entity type %s, it already exists", newTypeName.c_str());
        return;
    }

    sNameComponent* cloneTypeName = entity->getComponent<sNameComponent>();
    ASSERT(cloneTypeName != nullptr, "sNameComponent should exist");

    EntityFactory::createEntityType(newTypeName);

    EntityFactory::reverseAnimations(entity);

    // Save entity components
    auto& entityTypeInfos = EntityFactory::getInfos(cloneTypeName->value);
    const auto& entityTypeComponents = entity->getComponents();
    for (auto component: entityTypeComponents)
    {
        // Add cloned component to component factory
        std::string componentName = IComponentFactory::getComponentNameWithHash(component->id);
        auto compFactory = IComponentFactory::getFactory(componentName);
        sComponent* newComponent = component->clone();

        if (componentName == "sNameComponent")
        {
            sNameComponent* name = static_cast<sNameComponent*>(newComponent);
            name->value = newTypeName;
        }

        compFactory->save(newTypeName, newComponent);

        // Add component to EntityFactory
        // sNameComponent and sTransformComponent are already created in EntityFactory::createEntityType
        if (componentName != "sNameComponent" && componentName != "sTransformComponent")
        {
            EntityFactory::addComponent(newTypeName, componentName);
        }
    }

    EntityFactory::setTag(newTypeName, entityTypeInfos.tag);

    // Save the cloned template to json
    EntityFactory::saveEntityTemplateToJson(newTypeName);
}

void    LevelEntitiesDebugWindow::setSelectedEntityHandler(Entity::sHandle handle)
{
    _selectedEntityHandler = handle;
}

void    LevelEntitiesDebugWindow::populateInspector()
{
    EntityManager*  em;
    Entity*         selectedEntity;

    em = GameWindow::getInstance()->getGameStateManager()->getCurrentState()->getWorld().getEntityManager();
    if (em != nullptr)
    {
        selectedEntity = em->getEntity(this->_selectedEntityHandler);
        if (selectedEntity != nullptr)
            this->displayEntityDebug(em, selectedEntity);
    }
}
