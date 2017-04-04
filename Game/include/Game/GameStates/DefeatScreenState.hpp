/**
* @Author   Mathieu Chassara
*/

#include    <Engine/Core/GameState.hpp>

START_GAMESTATE(DefeatScreenState, "DefeatScreen")
 public:
     ~DefeatScreenState();

     void           onEnter() override final;
     void           setupSystems() override final;
     bool           init() override final;

private:
    Entity*         _buttonTryAgainRed = nullptr;

    END_GAMESTATE(DefeatScreenState)