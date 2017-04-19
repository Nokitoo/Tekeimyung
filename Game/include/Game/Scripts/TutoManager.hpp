/**
* @Author   Julien CHARDON
*/

#pragma once

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <Engine/Core/BaseScript.hpp>
#include <Engine/Core/ScriptFactory.hpp>

#define TUTO_MANAGER_TAG    "TutoManager"

enum eTutoStates
{
    MOVE = 0,
    SHOOT,
    CHANGE_WEAPON,
    CHOOSE_BUILD,
    BUILD,
    DISABLE_BUILD,
    CHECK_HOWTOPLAY,
    CHECK_BUILDLIST,
    TUTO_DONE,
    TUTO_HIDDEN
};

class TutoManager final : public BaseScript
{
public:
    TutoManager() = default;
    ~TutoManager() = default;

public:
    void start() override final;
    void update(float dt) override final;

private:
    EntityManager*  _em;
    const Entity*   _gameManager;
    const Entity*   _tutoDisplay;

    eTutoStates     _states;
};

REGISTER_SCRIPT(TutoManager);
