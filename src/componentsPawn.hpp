#pragma once

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





struct Walking { };
