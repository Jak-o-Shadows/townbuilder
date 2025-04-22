#include "componentsPawn.hpp"
#include "ticks.hpp"
#include "coordinates.hpp"

#include <random>

namespace Pawn {

// Handle extern entities
flecs::entity pawnsParent;
std::shared_ptr<spdlog::logger> logger;


module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<module>();
    logger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    logger->trace("Module Created");
    
    //ecs.import<Ticks::module>();  // TODO: Need to more registerModule out
    //ecs.import<Map::module>();

    
    // Modify pathfinding components
    // Each pawn can only occupy a single cell, so make exclusive
    ecs.component<PawnOccupying>().add(flecs::Exclusive);
    // Each pawn can only have a single target cell, so make exclusive
    ecs.component<PawnPathfindingGoal>().add(flecs::Exclusive);
    // Each pawn can only have a single next cell, so make exclusive
    ecs.component<PawnNextCell>().add(flecs::Exclusive);
    

   
    // Register components with reflection data & documentation
    ecs.component<PawnLifeTraits>()
        .member<float>("hunger")
        .member<float>("thirst")
        .member<float>("cold")
        .member<float>("comfort");
    ecs.component<PawnAbilityTraits>()
        .member<float>("strength")
        .member<float>("speed");
    logger->trace("Components Registered");
    
    // Need to give the entities a parent so they show nicer in the flecs explorer
    
    pawnsParent = ecs.entity("pawns");

    const Map::Grid* map = Map::mapEntity.get<Map::Grid>();
    
   
    // Put systems in
    auto move_sys = ecs.system<Coordinates::Cell, Coordinates::CellVelocity>("System_IntraCellMovement")
    .tick_source(Ticks::tick_pawn_behaviour)
    .run([](flecs::iter& it){
        ZoneScopedN("System_IntraCellMovement");
        while (it.next()){
            auto p = it.field<Coordinates::Cell>(0);
            auto v = it.field<Coordinates::CellVelocity>(1);
            for (auto i: it){
                p[i].x += v[i].x * it.delta_system_time();
                p[i].y += v[i].y * it.delta_system_time();
                //std::cout << p[i].x << ", " << p[i].y << " @ " << v[i].x << ", " << v[i].y << std::endl;
            }
        }
    });
    
    ecs.observer<const Coordinates::Grid>("Observer_PawnOccupying")
        .term_at(0).in()
        .event(flecs::OnSet)
        .each([&map](flecs::entity pawn, const Coordinates::Grid& grid){
            ZoneScopedN("Observer_PawnOccupying");
            pawn.add<PawnOccupying>(flecs::entity(pawn.world(), map->get(grid.x, grid.y)));
        });

    logger->trace("Module Setup Complete");



    // Generate pawns
    //  Randomly distribute starting & target positions
    std::mt19937 rng;
    rng.seed(20231104);
    std::uniform_int_distribution<int> xDist(0, map->m_width-1);
    std::uniform_int_distribution<int> yDist(0, map->m_height-1);
    std::uniform_real_distribution<float> speedDist(0.7, 0.9);

    constexpr int numPawns = 1;
    for (int pawnNumber=0; pawnNumber < numPawns; pawnNumber++){
        int targetX = xDist(rng);
        int targetY = yDist(rng);
        int myX = xDist(rng);
        int myY = yDist(rng);
        float speed = (float) speedDist(rng);
        char pawnName[200];
        sprintf(pawnName, "Pawn%d", pawnNumber);  // TODO: Replace with std::format
        auto pawn = ecs.entity(pawnName)
            .child_of(pawnsParent)
            .is_a<Pawn_Prefab>()
            .set<Coordinates::Grid>({myX, myY})
            .set<Coordinates::Cell>({0, 0})
            .set<Coordinates::CellVelocity>({0, 0})
            .add<Coordinates::GridBase>()
            .set<PawnAbilityTraits>({0, speed})
            .add<PawnOccupationWoodcutter>()
            .add<PawnWoodcutterState>(ecs.component<PawnWoodcutterStateIdle>());


        //PawnFSM::Instance machine{blah};
        //pawn.set<PawnFSMContainer>({PawnFSM::Instance{blah}});
        //std::unique_ptr<PawnFSM::Instance> ptr(new PawnFSM::Instance(blah));// = std::make_unique<PawnFSM::Instance>(machine);

        // Create the FSM
        LogicPawn::Context blah{pawn.id(), ecs};
        LogicPawn::PawnFSM::Instance test{blah};
        std::shared_ptr<LogicPawn::PawnFSM::Instance> ptr(&test);
        pawn.set<PawnFSMContainer>({ptr});

        //flecs::entity_to_json_desc_t desc;
        //desc.serialize_path = true;
        //desc.serialize_values = true;
        //std::cout << pawn.to_json(&desc) << "\n";
        std::shared_ptr<LogicPawn::PawnFSM::Instance> machine = pawn.get<PawnFSMContainer>()->machine;
        machine->changeTo<LogicPawn::Walking>();
        machine->update();

    }

    logger->trace("Created pawns");



};



}