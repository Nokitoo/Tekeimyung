#include <Systems/ProjectileSystem.hpp>
#include <Core/Components.hh>
#include <EntityFactory.hpp>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

ProjectileSystem::ProjectileSystem()
{
    addDependency<sProjectileComponent>();
    addDependency<sPositionComponent>();
    addDependency<sDirectionComponent>();
}

ProjectileSystem::~ProjectileSystem() {}

void    ProjectileSystem::update(EntityManager &em, float elapsedTime)
{
    forEachEntity(em, [&](Entity *entity)
    {
        sProjectileComponent*   component = entity->getComponent<sProjectileComponent>();

        if (component->guided == true)
        {
            Entity* target = em.getEntity(component->targetId);

            if (target != nullptr)
            {
                sPositionComponent* projectilePosition = entity->getComponent<sPositionComponent>();
                sPositionComponent* targetPosition = target->getComponent<sPositionComponent>();
                sDirectionComponent*    projectileDirection = entity->getComponent<sDirectionComponent>();

                //std::cout << "Target ID: " << component->targetId << std::endl;
                //std::cout << "Projectile position: " << glm::to_string(projectilePosition->value) << std::endl;
                //std::cout << "Target position: " << glm::to_string(targetPosition->value) << std::endl;
                projectileDirection->value = glm::normalize(targetPosition->value - projectilePosition->value) * projectileDirection->speed;
                //projectileDirection->speed = 100.0f;
            }
        }
    });
}
