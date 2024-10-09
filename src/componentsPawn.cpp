#include "componentsPawn.hpp"
#include "ticks.hpp"

#include <random>

namespace Pawn {

// Handle extern entities
flecs::entity pawnsParent;


module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    ecs.module<module>();
    
    ecs.import<Ticks::module>();

    
    // Modify pathfinding components
    // Each pawn can only occupy a single cell, so make exclusive
    ecs.component<PawnOccupying>().add(flecs::Exclusive);
    // Each pawn can only have a single target cell, so make exclusive
    ecs.component<PawnPathfindingGoal>().add(flecs::Exclusive);
    // Each pawn can only have a single next cell, so make exclusive
    ecs.component<PawnNextCell>().add(flecs::Exclusive);

    // Register components with reflection data & doc

    ecs.component<PawnLifeTraits>()
        .member<float>("hunger")
        .member<float>("thirst")
        .member<float>("cold")
        .member<float>("comfort");
    ecs.component<PawnAbilityTraits>()
        .member<float>("strength")
        .member<float>("speed");
    ecs.component<Pawn::Position>()
        .member<double>("x")
        .member<double>("y")
        .set_doc_brief("Location within the grid cell. Limited to [-1, 1]");
    ecs.component<Pawn::Velocity>()
        .member<double>("x")
        .member<double>("y");

    // Need to give the entities a parent so they show nicer in the flecs explorer
    pawnsParent = ecs.entity("pawns");

    const Map::Grid* map = Map::mapEntity.get<Map::Grid>();

    ecs.observer()
        .event(flecs::OnAdd)
        .with<PawnOccupying>(flecs::Wildcard)
        .with<PawnPathfindingGoal>(flecs::Wildcard)
        .each([map, &ecs](flecs::entity e){
            ZoneScopedN(ts_PawnPathfindingUpdate);
            //std::cout << "Pathfinding Update: for " <<  e.name() << std::endl;
            flecs::entity currentCell = e.target<PawnOccupying>();
            flecs::entity targetCell = e.target<PawnPathfindingGoal>();
            //std::cout << "\t " << currentCell.name() << " to " << targetCell.name() << std::endl;
            // From the current cell, and the target cell, pathfind to find the next cell
            //  First convert the entity data into x, y
            //      Current Cell
            auto currentStatic = currentCell.get<GridCellStatic>();
            int x = currentStatic->x;
            int y = currentStatic->y;
            //      Target Cell
            auto targetStatic = targetCell.get<GridCellStatic>();
            int targetX = targetStatic->x;
            int targetY = targetStatic->y;
            if ((targetX == x) && (targetY == y)) {
                // No need to pathfind - just move to the centre of the cell
                //  Jump to it - realistically the next level logic would take over and move to the right place
                Position& p = e.ensure<Position>();
                //std::cout << "\t" << e.name() << " finished!" << std::endl;
                p.x = 0;
                p.y = 0;
                e.set<Velocity>({0, 0});
                std::shared_ptr<LogicPawn::PawnFSM::Instance> machine = e.get<PawnFSMContainer>()->machine;
                LogicPawn::Arrived_Event ev;
                machine->react(ev);

            } else {
                //  Begin Pathfinding
                flecs::id_t nextCellId = pathfind(ecs, map, x, y, targetX, targetY);
                flecs::entity nextCell = flecs::entity(ecs, nextCellId);
                // Set the next cell
                e.add<PawnNextCell>(nextCell);
                // Calculate velocity to get to next cell
                // TODO: This might need to be smarter if I have non-uniform cells
                // TODO: This is assuming from middle of cell to middle of cell, not from edge to edge
                const PawnAbilityTraits *pawnAbilityTraits = e.get<PawnAbilityTraits>();
                float speed = pawnAbilityTraits->speed;
                auto nextStatic = nextCell.get<GridCellStatic>();
                float vx = (nextStatic->x - x)*speed;
                float vy = (nextStatic->y -y)*speed;
                //std::cout << "\t Velocity To: " << vx << ", " << vy << std::endl;
                e.set<Velocity>({vx, vy});
            }
        }).disable();
    

    // Generate pawns
    //  Randomly distribute starting & target positions
    std::mt19937 rng;
    rng.seed(20231104);
    std::uniform_int_distribution<int> xDist(0, map->m_width-1);
    std::uniform_int_distribution<int> yDist(0, map->m_height-1);
    std::uniform_real_distribution<float> speedDist(0.7, 0.9);

    constexpr int numPawns = 20;
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
            .set<Position>({0.5, 0.5})
            .set<Velocity>({0, 0})
            .set<PawnAbilityTraits>({0, speed})
            .add<PawnOccupationWoodcutter>()
            .add<PawnWoodcutterState>(ecs.component<PawnWoodcutterStateIdle>());
            //.add<PawnOccupying>(flecs::entity(ecs, map->get(myX,myY)))
            //.add<PawnPathfindingGoal>(flecs::entity(ecs, map->get(targetX, targetY)));


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


    // Put systems in
    auto move_sys = ecs.system<Position, Velocity>("System_Intra Cell Movement")
    .tick_source(Ticks::tick_pawn_behaviour)
    .run([](flecs::iter& it){
        while (it.next()){
            auto p = it.field<Position>(0);
            auto v = it.field<Velocity>(1);
            for (auto i: it){
                p[i].x += v[i].x * it.delta_system_time();
                p[i].y += v[i].y * it.delta_system_time();
                //std::cout << p[i].x << ", " << p[i].y << " @ " << v[i].x << ", " << v[i].y << std::endl;
            }
        }
    });
    
    auto moveCell_sys = ecs.system<Position>("System Between Cell Movement")
    .with<PawnNextCell>(flecs::Wildcard)
    .tick_source(Ticks::tick_pawn_behaviour)
    .multi_threaded()
    .each([](flecs::entity e, Position& p){
        ZoneScopedN("System Between Cell Movement");
        //std::cout << "Checking Cell Moveover" << std::endl;
        bool movedCell = false;
        // Assume that the velocity is correct, and if we move over any boundary we're there
        if (p.x > 1){
            movedCell = true;
            p.x = -1;
        } else if (p.x < -1) {
            // Moved cell left
            movedCell = true;
            p.x = 1;
        } 
        if (p.y > 1) {
            // Moved cell up
            movedCell = true;
            p.y = -1;
        } else if (p.y < -1) {
            // Moved cell down
            movedCell = true;
            p.y = 1;
        }

        if (movedCell){
            flecs::entity nextCell = e.target<PawnNextCell>();
            //std::cout << " Moved To " << nextCell.name() << std::endl;
            // Uupdate currently occupying cell
            e.add<PawnOccupying>(nextCell);
            // Update overall goal to force pathfinding update
            flecs::entity goalCell = e.target<PawnPathfindingGoal>();
            e.add<PawnPathfindingGoal>(goalCell);
        }
    }).disable();








};



}