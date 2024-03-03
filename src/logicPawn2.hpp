#pragma once

#include <flecs.h>

// optional: enable FSM structure report in debugger
#define HFSM2_ENABLE_ALL
#include <hfsm2/machine.hpp>

#include <iostream>

//------------------------------------------------------------------------------

// data shared between FSM states and outside code
//struct Context {
//	unsigned cycleCount = 0;
//};
using Context = flecs::id_t;

// convenience typedef
using M = hfsm2::MachineT<hfsm2::Config::ContextT<Context>>;



// states need to be forward declared to be used in FSM struct declaration.
//    This also allows them to be used as tags in flecs
struct Alive;
struct Idle;
struct Working;
struct Fleeing;
struct Combat;
struct Dead;

using FSM = M::PeerRoot<
				// sub-machine ..
				M::Composite<Alive,
					// .. with 4 sub-states
					Idle,
					Working,
					Fleeing,
					Combat
				>,
				Dead
			>;
