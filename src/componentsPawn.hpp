#pragma once

#include <flecs.h>
#include <tracy/Tracy.hpp>

#include "tracy_zones.hpp"

#include "gridMap.hpp"
#include "logicPawn2.hpp"

namespace Pawn{

struct module {
    module(flecs::world& ecs);
};

extern flecs::entity pawnsParent;


struct PawnLifeTraits {
    float hunger;
    float thirst;
    float cold;
    float comfort;
};

struct PawnAbilityTraits {
    float strength;
    float speed;
};

struct PawnPathfindingGoal {};

struct PawnOccupying {};

struct PawnNextCell {};

struct Position {
    double x, y;
};

struct Velocity {
    double x, y;
};


struct Likes { };


struct PawnFSMContainer {
    std::unique_ptr<LogicPawn::PawnFSM::Instance> machine;
};



// Pawn Occupations
struct PawnOccupationUnemployed {};
struct PawnOccupationWoodcutter {};



// Pawn States
//  Woodcutter
struct PawnWoodcutterState {};
struct PawnWoodcutterStateIdle {};
struct PawnWoodcutterStateWalkingTo {};
struct PawnWoodcutterStateReturning {};
struct PawnWoodcutterStateChopping {};


}