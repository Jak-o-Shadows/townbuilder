#pragma once

#include <flecs.h>
#include <tracy/Tracy.hpp>

#include "tracy_zones.hpp"

#include "gridMap.hpp"
#include "logicPawn2.hpp"
#include "msgLogging.hpp"

namespace Pawn{

struct module {
    module(flecs::world& ecs)
;};

extern flecs::entity pawnsParent;

struct Pawn_Prefab {};
struct IsAPawn {};

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

struct Likes { };


struct PawnFSMContainer {
    std::shared_ptr<LogicPawn::PawnFSM::Instance> machine;
};





}