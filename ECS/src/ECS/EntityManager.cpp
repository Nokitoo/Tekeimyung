/**
* @Author   Guillaume Labey
*/

#include <algorithm>
#include <random>

#include <ECS/World.hpp>

#include <ECS/EntityManager.hpp>

int                         EntityManager::_entityId = 1;


EntityManager::EntityManager(World& world): _world(world) {}

EntityManager::~EntityManager() {}

Entity* EntityManager::createEntity()
{
    Entity* entity = new Entity(this, _entityId++);

    _entities[entity->id] = entity;
    return (entity);
}

void    EntityManager::destroyEntity(Entity* entity)
{
    _world.notifyEntityDeleted(entity);
    std::for_each(entity->_components.begin(), entity->_components.end(), [](sComponent* component)
    {
        delete component;
    });
    entity->_components.clear();

    _entities.erase(entity->id);
}

void    EntityManager::destroyEntityRegister(Entity* entity)
{
    _entitiesToDestroy.push_back(entity);
}

void    EntityManager::destroyEntities()
{
    if (_entitiesToDestroy.empty())
        return;

    for (auto entity : _entitiesToDestroy)
    {
        destroyEntity(entity);
    }
    _entitiesToDestroy.clear();
}

void    EntityManager::destroyAllEntities()
{
    auto it = _entities.begin();
    for (it; it != _entities.end();)
    {
        Entity* entity = it->second;
        ++it;
        destroyEntity(entity);
    }
    _entitiesToDestroy.clear();
}

std::unordered_map<uint32_t, Entity*>& EntityManager::getEntities()
{
    return (_entities);
}

const std::vector<Entity*>& EntityManager::getEntitiesByTag(const std::string& tag)
{
    return (_entitiesTagGroups[tag]);
}

Entity* EntityManager::getEntity(uint32_t id) const
{
    auto &&it = _entities.find(id);

    return (it != _entities.end() ? it->second : nullptr);
}

void    EntityManager::notifyEntityNewComponent(Entity* entity, sComponent* component)
{
    _world.notifyEntityNewComponent(entity, component);
}

void    EntityManager::notifyEntityRemovedComponent(Entity* entity, sComponent* component)
{
    _world.notifyEntityRemovedComponent(entity, component);
}

void    EntityManager::notifyEntityCreated(Entity* entity)
{
    _world.notifyEntityCreated(entity);
}

void    EntityManager::addEntityToTagGroup(Entity* entity, const std::string& name)
{
    if (name.size() == 0)
        return;

    _entitiesTagGroups[name].push_back(entity);
}

void    EntityManager::removeEntityFromTagGroup(Entity* entity, const std::string& name)
{
    if (name.size() == 0)
        return;

    auto& tagGroup = _entitiesTagGroups.find(name);
    if (tagGroup != _entitiesTagGroups.end())
    {
        tagGroup->second.push_back(entity);
    }
}
