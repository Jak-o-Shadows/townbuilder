#pragma once
#include "pythonEcsBinding/module.hpp"

#include <tuple>

// Include all component headers here
#include "msgLogging/module.hpp"
//#include "render/module.hpp"
#include "coordinates/module.hpp"
//#include "dis/module.hpp"
#include "pawn/module.hpp"
//#include "map/module.hpp"
#include "buildings/module.hpp"
#include "plugin/module.hpp"
//#include "pathfinding/module.hpp"
#include "ui/module.hpp"
//#include "statemachine/module.hpp"

/**
 * @brief A type list of all components to be exposed to Python.
 *
 * Add every component struct you want to bind to this tuple. The automation
 * script will handle the rest by using Flecs' reflection data.
 */
using AllComponents = std::tuple<
    Plugin::PluginGuid
    //Buildings::Resources, Buildings::BuildingUI,
    //Coordinates::Grid, Coordinates::Cell, Coordinates::CellVelocity, Coordinates::GridBase, Coordinates::NedBase, Coordinates::LlaBase, Coordinates::EcefBase, Coordinates::NED, Coordinates::LLA, Coordinates::ECEF,
    //fdis::
    //Logging::LoggerControls,
    //Pawn::PawnLifeTraits, Pawn::PawnAbilityTraits, 
    //UI::PawnJobs
    >;
//    Pawn::PawnLifeTraits, Pawn::PawnAbilityTraits,
//    Python::PythonFile
//>;