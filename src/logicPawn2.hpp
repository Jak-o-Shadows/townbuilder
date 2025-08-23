#pragma once

#include "msgLogging.hpp"


#include <flecs.h>

// optional: enable FSM structure report in debugger
#define HFSM2_ENABLE_ALL
#include <hfsm2/machine.hpp>

#include <iostream>
#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

//------------------------------------------------------------------------------

namespace LogicPawn {

extern std::shared_ptr<spdlog::logger> logger;  // Declaration for use in base class and elsewhere, as opposed to the normal way

// From https://stackoverflow.com/questions/1055452/c-get-name-of-type-in-template
namespace loggingImpl
{
    template <typename T>
    [[nodiscard]] constexpr std::string_view RawTypeName()
    {
        #ifndef _MSC_VER
        return __PRETTY_FUNCTION__;
        #else
        return __FUNCSIG__;
        #endif
    }

    struct TypeNameFormat
    {
        std::size_t junk_leading = 0;
        std::size_t junk_total = 0;
    };

    constexpr TypeNameFormat type_name_format = []{
        TypeNameFormat ret;
        std::string_view sample = RawTypeName<int>();
        ret.junk_leading = sample.find("int");
        ret.junk_total = sample.size() - 3;
        return ret;
    }();
    static_assert(type_name_format.junk_leading != std::size_t(-1), "Unable to determine the type name format on this compiler.");

    template <typename T>
    static constexpr auto type_name_storage = []{
        std::array<char, RawTypeName<T>().size() - type_name_format.junk_total + 1> ret{};
        std::copy_n(RawTypeName<T>().data() + type_name_format.junk_leading, ret.size() - 1, ret.data());
        return ret;
    }();
}

template <typename T>
[[nodiscard]] constexpr std::string_view TypeName()
{
    return {loggingImpl::type_name_storage<T>.data(), loggingImpl::type_name_storage<T>.size() - 1};
}

template <typename T>
[[nodiscard]] constexpr const char *TypeNameCstr()
{
    return loggingImpl::type_name_storage<T>.data();
}


struct Context {
	flecs::id_t id;
	flecs::world& ecs;
};

struct StateTiming{
	float timeInState_s = 0;
	float culmulativeTimeInState_s = 0;
};

// data shared between FSM states and outside code

	// convenience typedef
	using M = hfsm2::MachineT<hfsm2::Config::ContextT<Context>>;


	// Events
	struct Destination_Event{
		int x;
		int y;
	};
	struct Arrived_Event {};  // When you arrive at a location or cell
	struct SecondaryEvent { int payload; };
	struct Attacked {};



	// states need to be forward declared to be used in FSM struct declaration.
	//    This also allows them to be used as tags in flecs
	struct Alive;
	struct Idle;
	struct Walking;
	struct Working;
	struct Fleeing;
	struct Combat;
	struct Dead;

	// Job & Job Related
	
	// Pawn Occupations
	struct PawnOccupationUnemployed;
	struct PawnOccupationWoodcutter;


	// Pawn States
	//  Woodcutter
	struct PawnWoodcutterStateWalkingTo;
	struct PawnWoodcutterStateReturning;
	struct PawnWoodcutterStateChopping;





	using PawnFSM = M::PeerRoot<
					// sub-machine ..
					M::Composite<Alive,
						// .. with 4 sub-states
						Idle,
						M::Composite<Working,
							PawnOccupationUnemployed,
							M::Utilitarian<PawnOccupationWoodcutter,
								PawnWoodcutterStateWalkingTo,
								PawnWoodcutterStateReturning,
								PawnWoodcutterStateChopping
								>
							>,
						Walking,
						Fleeing,
						Combat
					>,
					Dead
				>;


	template <typename TemplateState>
	struct BasePawnState : PawnFSM::State {
		// BasePawnState is a base class for all Pawn states, providing default reactions
		void react(const Destination_Event&, FullControl& control) {};
		void react(const Arrived_Event&, FullControl& control) {};
		void react(const SecondaryEvent&, FullControl& control) {};
		void react(const Attacked&, FullControl& control) {};
		void enter(Control& control) {
			flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
			logger->trace("Pawn {} entering state {}", std::string(e.path()), TypeName<TemplateState>());
			// Reset how long we've been in this state
			StateTiming* timing = e.get_mut<StateTiming, TemplateState>();
			timing->timeInState_s = 0;
			e.add<TemplateState>();
		}
		void exit(Control& control) {
			flecs::entity e = flecs::entity(control.context().ecs, control.context().id);
			logger->trace("Pawn {} exiting state {}", std::string(e.path()), TypeName<TemplateState>());
			e.remove<TemplateState>();
		}
	};


	// Overall Pawn States

	struct Alive : BasePawnState<Alive> {
	};

	struct Idle : BasePawnState<Idle> {
		void react(const Destination_Event& event, FullControl& control);
	};

	struct Working : BasePawnState<Working> {
	};

	struct Walking : BasePawnState<Walking> {
		void react(const Destination_Event& event, FullControl& control);
		void react(const Arrived_Event&, FullControl& control);
	};

	struct Combat : BasePawnState<Combat> {
		void update(FullControl& control) {};
	};

	struct Fleeing : BasePawnState<Fleeing> {
	};

	struct Dead : BasePawnState<Dead> {
	};


	// Occupation Pawn States
	struct PawnOccupationUnemployed : BasePawnState<PawnOccupationUnemployed> {
	};

	struct PawnOccupationWoodcutter : BasePawnState<PawnOccupationWoodcutter> {
	};

	struct PawnWoodcutterStateWalkingTo : BasePawnState<PawnWoodcutterStateWalkingTo> {
		void react(const Arrived_Event&, FullControl& control);
	};

	struct PawnWoodcutterStateReturning : BasePawnState<PawnWoodcutterStateReturning> {
	};

	struct PawnWoodcutterStateChopping : BasePawnState<PawnWoodcutterStateChopping> {
	};




struct module {
    module(flecs::world& ecs);


	//



};

}