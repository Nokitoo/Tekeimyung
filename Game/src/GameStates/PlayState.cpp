#include "Graphics/Systems/RenderingSystem.hpp"
#include "Core/Components.hh"

#include "GameStates/PlayState.hpp"


PlayState::PlayState() {}

PlayState::~PlayState() {}

bool    PlayState::init()
{
    EntityManager &em = _world.getEntityManager();

    _world.addSystem<RenderingSystem>();

    Entity *entity = em.createEntity();
    entity->addComponent<sPositionComponent>(9, 9, 3);
    entity->addComponent<sDirectionComponent>(0, 0);
    entity->addComponent<sRenderComponent>(eSpriteType::OBJECT, "ressources/sprites/taxi.png");

    _map.resize(3);
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            entity = em.createEntity();
            entity->addComponent<sPositionComponent>(x, y, 0);
            entity->addComponent<sRenderComponent>(eSpriteType::TILE, "ressources/sprites/Landscape/landscape_13.png");
            _map[0][y][x] = entity;
        }
    }
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            entity = em.createEntity();
            entity->addComponent<sPositionComponent>(x, y, 2);
            entity->addComponent<sRenderComponent>(eSpriteType::TILE, "ressources/sprites/Landscape/landscape_30.png");
            _map[2][y][x] = entity;
        }
    }
   for (int y = 0; y < 7; y++) {
        for (int x = 0; x < 7; x++) {
            entity = em.createEntity();
            entity->addComponent<sPositionComponent>(x, y, 1);
            entity->addComponent<sRenderComponent>(eSpriteType::TILE, "ressources/sprites/Landscape/landscape_33.png");
            _map[1][y][x] = entity;
        }
    }

    // Sort entities with their depth value
    em.sortEntities([](Entity *first, Entity *second) {
        unsigned int depthFirst = 0;
        unsigned int depthSecond = 0;
        sPositionComponent *positionFirst = first->getComponent<sPositionComponent>();
        sPositionComponent *positionSecond = second->getComponent<sPositionComponent>();

        if (positionFirst)
        {
            depthFirst = positionFirst->x + positionFirst->y + positionFirst->z;
        }
        if (positionSecond)
        {
            depthSecond = positionSecond->x + positionSecond->y + positionSecond->z;
        }

        return depthFirst < depthSecond;
    });





    return (true);
}