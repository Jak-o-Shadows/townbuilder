#pragma once

#include <flecs.h>

// optional: enable FSM structure report in debugger
#define HFSM2_ENABLE_ALL
#include <hfsm2/machine.hpp>

#include <iostream>

//------------------------------------------------------------------------------

namespace LogicPawn {

struct Context {
	flecs::id_t id;
	flecs::world& ecs;
};

// data shared between FSM states and outside code

	// convenience typedef
	using M = hfsm2::MachineT<hfsm2::Config::ContextT<Context>>;


	// Events
	struct Arrived_Event {};
	//struct SecondaryEvent { int payload; };



	// states need to be forward declared to be used in FSM struct declaration.
	//    This also allows them to be used as tags in flecs
	struct Alive;
	struct Idle;
	struct Working;
	struct Walking;
	struct Fleeing;
	struct Combat;
	struct Dead;

	using PawnFSM = M::PeerRoot<
					// sub-machine ..
					M::Composite<Alive,
						// .. with 4 sub-states
						Idle,
						Working,
						Walking,
						Fleeing,
						Combat
					>,
					Dead
				>;

	struct Alive:PawnFSM::State {
		void enter(Control& control);
		void exit(Control& control);
	};

	struct Idle:PawnFSM::State {
		void enter(Control& control);
		void exit(Control& control);
		void update(FullControl& control);
	};

	struct Working:PawnFSM::State {
		void enter(Control& control);
		void exit(Control& control);
		void update(FullControl& control);
	};

	struct Walking:PawnFSM::State {
		void enter(Control& control);
		void exit(Control& control);
		void react(const Arrived_Event&, FullControl& control);
	};

	struct Combat:PawnFSM::State {
		void enter(Control& control);
		void exit(Control& control);
		void update(FullControl& control);
	};

	struct Fleeing:PawnFSM::State {
		void enter(Control& control);
		void exit(Control& control);
		void update(FullControl& control);
	};

	struct Dead:PawnFSM::State{
		void enter(Control& control);
		void exit(Control& control);
	};


struct module {
    module(flecs::world& ecs);
};

}