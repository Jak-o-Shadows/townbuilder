//#define TRACY_ON_DEMAND




#include "tracy_zones.hpp"

#include <flecs.h>
#include <tracy/Tracy.hpp>




// Tracy memory tracking
void* operator new(std::size_t count) {
    auto ptr = malloc(count);
    TracyAlloc(ptr, count);
    return ptr;
}
void operator delete(void* ptr) noexcept {
    TracyFree(ptr);
    free(ptr);
}


#include <iostream>
#include <vector>
#include <random>

























































































int main(int, char *[]) {

    flecs::world ecs;
    ecs.set<flecs::Rest>({});
    ecs.import<flecs::monitor>(); // Enable statistics in explorer






    // Create basic timers
    // 1000 Hz should be enough for anybody
    flecs::entity tick_100_Hz = ecs.timer("Timer_100 Hz")
        .interval(0.01);
    // Pawn behaviour
    flecs::entity tick_pawn_behaviour = ecs.timer("Timer_Pawn Behaviour")
        .rate(4, tick_100_Hz);  // 4 ticks @ 100 Hz => 25 Hz
    // UI Updates
    flecs::entity tick_ui = ecs.timer("Timer_UI Update")
        .rate(8, tick_100_Hz);  // 8 ticks @ 100 Hz => 12.5 Hz



















    // Frame Markers for Tracy
    if (true) {
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

    ecs.set_threads(4);
    while(ecs.progress(0)){
        FrameMarkNamed("Flecs Update");
    };
}