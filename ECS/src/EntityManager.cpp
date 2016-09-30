#include <algorithm>
#include "EntityManager.hpp"
#include <algorithm>
#include <random>

int                         EntityManager::_entityId = 0;


EntityManager::EntityManager() {}

EntityManager::~EntityManager() {}

Entity* EntityManager::createEntity()
{
    Entity* entity = new Entity(_entityId++);

    _entities.push_back(entity);
    return (entity);
}

void destroy()
{

}

void    EntityManager::destroyEntity(Entity* entity)
{
    std::for_each(entity->_components.begin(), entity->_components.end(), [](sComponent* component)
    {
        delete component;
    });
    entity->_components.clear();

    for (auto it = _entities.begin(); it != _entities.end(); )
    {
        if (*(*it) == *entity)
        {
            delete *it;
            it = _entities.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

std::list<Entity*>&     EntityManager::getEntities()
{
    return (_entities);
}