#include "ticks.hpp"

#include <tracy/Tracy.hpp>


namespace Ticks{

flecs::entity tick_100_Hz;
flecs::entity tick_pawn_behaviour;
flecs::entity tick_ui;
flecs::entity tick_render;


module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    ecs.module<module>();

    // Create basic timers
    // 1000 Hz should be enough for anybody
    tick_100_Hz = ecs.timer("Timer_100 Hz")
        .interval(0.01);
    // Pawn behaviour
    tick_pawn_behaviour = ecs.timer("Timer_Pawn Behaviour")
        .rate(4, tick_100_Hz);  // 4 ticks @ 100 Hz => 25 Hz
    // UI Updates
    tick_ui = ecs.timer("Timer_UI Update")
        .rate(8, tick_100_Hz);  // 8 ticks @ 100 Hz => 12.5 Hz
    // Render update
    tick_render = ecs.timer("Timer_Render Update")
        .rate(2, tick_100_Hz);  // 2 ticks @ 100 Hz => 50 Hz

    // Frame Markers for Tracy
    if (true) {

        ecs.system("raw")
            .iter([](flecs::iter it){
                FrameMarkNamed("EverFrame");
        });

        ecs.system("Tracy 100 Hz Frame")
            .kind(flecs::OnUpdate)
            .tick_source(tick_100_Hz)
            .iter([](flecs::iter& it) {
                FrameMarkNamed("100 Hz");
        });

        ecs.system("Tracy Pawn Behaviour Frame")
            .kind(flecs::OnUpdate)
            .tick_source(tick_pawn_behaviour)
            .iter([](flecs::iter& it) {
                FrameMarkNamed("Pawn Behaviour");
        });

        ecs.system("Tracy UI Frame")
            .kind(flecs::OnUpdate)
            .tick_source(tick_ui)
            .iter([](flecs::iter& it) {
                FrameMarkNamed("Tick UI");
        });
    }



    };
}

