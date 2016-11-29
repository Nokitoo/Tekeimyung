#pragma once

#include <ECS/System.hpp>

#include <Engine/Utils/MonitoringDebugWindow.hpp>

#include <Game/Map.hpp>

#define COLLISION_SYSTEM_NAME "Collision system"

START_SYSTEM(CollisionSystem)
public:
    CollisionSystem(Map* map);
    virtual ~CollisionSystem() {};
    virtual void    update(EntityManager &em, float elapsedTime);

private:
    bool            isColliding(Entity *firstEntity, Entity *secondEntity);
    bool            staticResolution(Entity *firstEntity);
    bool            dynamicResolution(EntityManager &em, Entity *firstEntity, Entity *secondEntity);

private:
    Map*            _map;
    uint16_t        _keyMonitoring;
    tMonitoring     _data;
END_SYSTEM(CollisionSystem)
