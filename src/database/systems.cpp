#include "database/module.hpp"

#include <tracy/Tracy.hpp>

#include "msgLogging/module.hpp"
#include "ticks/module.hpp"
#include "pawn/module.hpp"
#include "statemachine/module.hpp"
#include "async_system.hpp"

#include <soci/sqlite3/soci-sqlite3.h>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <cstdio> // For std::remove
#include <future>
#include <fstream>

namespace Database {

std::shared_ptr<spdlog::logger> systemsLogger;

/**
 * @brief Writes a memory buffer to a file.
 * This is intended to be run in a background thread.
 */

void save_buffer_to_file(const std::string& destFilename, const std::shared_ptr<std::vector<unsigned char>> buffer) {
    ZoneScopedN("save_buffer_to_file");
    systemsLogger->debug("Saving database to file {}", destFilename);
    try {
        std::ofstream of(destFilename, std::ios::binary);
        of.exceptions(std::ios::failbit | std::ios::badbit);
        of.write(reinterpret_cast<const char*>(buffer->data()), buffer->size());
        systemsLogger->debug("Database successfully saved to {}", destFilename);
    } catch (const std::ios_base::failure& e) {
        systemsLogger->error("Failed to write database to file {}: {}", destFilename, e.what());
    }
}

/**
 * @brief Serializes an in-memory SQLite database into a memory buffer.
 * 
 * @param sourceSession The in-memory SOCI session to serialize.
 * @return A shared_ptr to a vector containing the serialized database.
 */
std::shared_ptr<std::vector<unsigned char>> serialize_database(soci::session& sourceSession) {
    ZoneScopedN("serialize_database");
    systemsLogger->debug("Serializing database");
    soci::sqlite3_session_backend* sourceBackend = static_cast<soci::sqlite3_session_backend*>(sourceSession.get_backend());
    sqlite_api::sqlite3* pFrom = sourceBackend->conn_;

    sqlite3_int64 size = 0;
    unsigned char* pData = sqlite3_serialize(reinterpret_cast<sqlite3*>(pFrom), "main", &size, 0);

    if (!pData) {
        systemsLogger->error("Failed to serialize database.");
        return nullptr;
    }

    auto buffer = std::make_shared<std::vector<unsigned char>>(pData, pData + size);
    sqlite3_free(pData);
    systemsLogger->debug("Database successfully serialized");
    return buffer;
}

std::tuple<Snapshot> gather_database_snapshot(flecs::world&, const Connection& conn) {
    systemsLogger->debug("Creating database snapshot for async save.");
    std::shared_ptr<std::vector<unsigned char>> buffer = serialize_database(*conn.sql);
    std::string dest_filename = "database_backup.sqlite3";
    return std::make_tuple(Snapshot({buffer, dest_filename}));
}

std::tuple<> work_save_database_snapshot(const Snapshot& snapshot) {
    if (snapshot.buffer) {  // TODO: Is this really neccessary? If it's empty, why not just write it
        systemsLogger->trace("Saving database snapshot to file.");
        save_buffer_to_file(snapshot.destination_filename, snapshot.buffer);
    }
    return std::make_tuple();
}


systems::systems(flecs::world& ecs) {
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Database systems module created");

    // This observer opens the database connection when the Connection component is set.
    ecs.observer<Connection>("Observer_OpenDatabaseConnection")
        .event(flecs::OnSet)
        .each([](Connection& conn) {
            systemsLogger->trace("Creating in-memory database connection");
            const char* db_connection_string = ":memory:";

            // Create and assign the new session
            try {
                conn.sql = std::make_shared<soci::session>(soci::sqlite3, db_connection_string);
                systemsLogger->info("In-memory database established.");

                // Enable Write-Ahead Logging.
                *conn.sql << "PRAGMA journal_mode=WAL;";
                systemsLogger->info("SQLite journal_mode set to WAL.");
            } catch (const std::exception& e) {
                systemsLogger->error("Failed to open in-memory database connection: {}", e.what());
                conn.sql = nullptr;
            }
        });

    

    
   // Use the `create_async_system` to periodically flush the database to disk via copying the in-memory SQLITE,
   //   then writing it to disk
   //   Note that a custom "tick source" is required because `create_async_system` doesn't support the `.interval`,
   //   because it is quite clumsily using flecs, rather than being more native
   flecs::entity flush_tick_source = ecs.timer("flush_log").interval(10);

   
    // Use the new async system builder to periodically save the database.
    std::function<std::tuple<Snapshot>(flecs::world&, const Connection&)>
        gather_database_snapshot_fn = gather_database_snapshot;

    std::function<std::tuple<>(const Snapshot&)>
        work_save_database_snapshot_fn = work_save_database_snapshot;

    Async::create_async_system_for_singleton(ecs, "SaveDatabaseSnapshot")
        .query<const Connection>() // Query for the singleton
        .tick_source(flush_tick_source)
        .work(work_save_database_snapshot_fn)
        .gather(gather_database_snapshot_fn)
        // No .apply() needed, the default does nothing for an empty results tuple.
        .build();

    /////////////// Pawn ///////////////////////////////

    // System to create the pawn_state_utility table on startup
    ecs.system<Database::Connection>("CreateTable_PawnStateUtility")
        .kind(flecs::OnStart)
        .each([](Database::Connection& conn) {
            try {
                conn.sql->create_table("pawn_state_utility")
                    .column("time", soci::dt_double)
                    .column("pawn_name", soci::dt_string)
                    .column("state_name", soci::dt_string)
                    .column("utility", soci::dt_double);
                systemsLogger->info("Table 'pawn_state_utility' created.");
            } catch (const std::exception& e) {
                systemsLogger->error("Error creating table 'pawn_state_utility': {}", e.what());
            }
        });
    
    ecs.system<const Statemachine::StateUtility, Database::Connection>("LogPawnStateUtility")
        .term_at(0).in()
        .term_at(0).second(flecs::Wildcard)
        .with<Pawn::PawnFSMContainer>()
        .tick_source(Ticks::tick_pawn_behaviour)
        .run([](flecs::iter& it) {
            ZoneScopedN("LogPawnStateUtility");
            systemsLogger->trace("Logging state utilities for {} pawns for a single state", it.count());
            while (it.next()) {
                auto state_utils = it.field<const Statemachine::StateUtility>(0);
                auto db_conn = it.field<Database::Connection>(1);

                // Use a single transaction for all the pawn inserts for efficiency
                //  Note that this system runs per StateUtility Second, so the system runs once per State, not
                //  once
                soci::transaction tr(*(db_conn->sql));
                for (auto i : it) {
                    double time = it.world().get_info()->world_time_total;
                    flecs::entity pawn = it.entity(i);
                    std::string pawn_name = std::string(pawn.path());
                    std::string state_name = std::string(it.pair(0).second().name());
                    systemsLogger->trace("Pawn: {}, State: {}, Utility: {}", 
                        pawn_name, state_name, state_utils[i].utility);

                    double utility = static_cast<double>(state_utils[i].utility);

                    *db_conn->sql << "INSERT INTO pawn_state_utility (time, pawn_name, state_name, utility) "
                                    "VALUES (:time, :pawn, :state, :util)",
                                    soci::use(time, "time"),
                                    soci::use(pawn_name, "pawn"),
                                    soci::use(state_name, "state"),
                                    soci::use(utility, "util");
                }
                tr.commit();
            }
        });

    // System to create the pawn current table on startup
    ecs.system<Database::Connection>("CreateTable_PawnActiveStates")
        .kind(flecs::OnStart)
        .each([](Database::Connection& conn) {
            try {
                conn.sql->create_table("pawn_active_states")
                    .column("time", soci::dt_double)
                    .column("pawn_name", soci::dt_string)
                    .column("Alive", soci::dt_integer)
                    .column("Idle", soci::dt_integer)
                    .column("Working", soci::dt_integer)
                    .column("PawnOccupationUnemployed", soci::dt_integer)
                    .column("PawnOccupationWoodcutter", soci::dt_integer)
                    .column("PawnWoodcutterStateWalkingTo", soci::dt_integer)
                    .column("PawnWoodcutterStateReturning", soci::dt_integer)
                    .column("PawnWoodcutterStateChopping", soci::dt_integer)
                    .column("Walking", soci::dt_integer)
                    .column("Fleeing", soci::dt_integer)
                    .column("Combat", soci::dt_integer)
                    .column("Dead", soci::dt_integer)
                    ;
                systemsLogger->info("Table 'pawn_active_states' created.");
            } catch (const std::exception& e) {
                systemsLogger->error("Error creating table 'pawn_active_states': {}", e.what());
            }
        });    

    ecs.system<Pawn::PawnFSMContainer, Database::Connection>("LogPawnStateActive")
        .tick_source(Ticks::tick_pawn_behaviour)
        .run([](flecs::iter& it) {
            ZoneScopedN("LogPawnStateActive");
            systemsLogger->trace("Logging active states for {} pawns", it.count());
            while (it.next()) {
                auto fsmc = it.field<Pawn::PawnFSMContainer>(0);
                auto db_conn = it.field<Database::Connection>(1);

                // Use a single transaction for all the pawn inserts for efficiency               
                soci::transaction tr(*(db_conn->sql));
                double time = it.world().get_info()->world_time_total;
                for (auto i : it) {
                    flecs::entity pawn = it.entity(i);
                    std::string pawn_name = std::string(pawn.path());

                    // SOCI can't take the values directly inline, so must assign to variables first
                    int alive =                static_cast<int>(fsmc->machine->isActive<Pawn::Alive>());
                    int idle =                 static_cast<int>(fsmc->machine->isActive<Pawn::Idle>());
                    int working =              static_cast<int>(fsmc->machine->isActive<Pawn::Working>());
                    int unemployed =           static_cast<int>(fsmc->machine->isActive<Pawn::PawnOccupationUnemployed>());
                    int woodcutter =           static_cast<int>(fsmc->machine->isActive<Pawn::PawnOccupationWoodcutter>());
                    int woodcutter_walkingto = static_cast<int>(fsmc->machine->isActive<Pawn::PawnWoodcutterStateWalkingTo>());
                    int woodcutter_returning = static_cast<int>(fsmc->machine->isActive<Pawn::PawnWoodcutterStateReturning>());
                    int woodcutter_chopping =  static_cast<int>(fsmc->machine->isActive<Pawn::PawnWoodcutterStateChopping>());
                    int walking =              static_cast<int>(fsmc->machine->isActive<Pawn::Walking>());
                    int fleeing =              static_cast<int>(fsmc->machine->isActive<Pawn::Fleeing>());
                    int combat =               static_cast<int>(fsmc->machine->isActive<Pawn::Combat>());
                    int dead =                 static_cast<int>(fsmc->machine->isActive<Pawn::Dead>());

                    *db_conn->sql << "INSERT INTO pawn_active_states (time, pawn_name, Alive, Idle, Working, PawnOccupationUnemployed, PawnOccupationWoodcutter, PawnWoodcutterStateWalkingTo, PawnWoodcutterStateReturning, PawnWoodcutterStateChopping, Walking, Fleeing, Combat, Dead) "
                                    "VALUES (:time, :pawn, :alive, :idle, :working, :unemployed, :woodcutter, :woodcutter_walkingto, :woodcutter_returning, :woodcutter_chopping, :walking, :fleeing, :combat, :dead)",
                                    soci::use(time, "time"),
                                    soci::use(pawn_name, "pawn"),
                                    soci::use(alive, "alive"),
                                    soci::use(idle, "idle"),
                                    soci::use(working, "working"),
                                    soci::use(unemployed, "unemployed"),
                                    soci::use(woodcutter, "woodcutter"),
                                    soci::use(woodcutter_walkingto, "woodcutter_walkingto"),
                                    soci::use(woodcutter_returning, "woodcutter_returning"),
                                    soci::use(woodcutter_chopping, "woodcutter_chopping"),
                                    soci::use(walking, "walking"),
                                    soci::use(fleeing, "fleeing"),
                                    soci::use(combat, "combat"),
                                    soci::use(dead, "dead");
                }
                tr.commit();
            }
        });

    ////////////////// Coordinates ////////////////////////
        ecs.system<Database::Connection>("CreateTable_entity_map_position")
        .kind(flecs::OnStart)
        .each([](Database::Connection& conn) {
            try {
                conn.sql->create_table("entity_map_position")
                    .column("time", soci::dt_double)
                    .column("entity_name", soci::dt_string)
                    .column("Cell_x", soci::dt_double)
                    .column("Cell_y", soci::dt_double)
                    .column("Grid_x", soci::dt_integer)
                    .column("Grid_y", soci::dt_integer)
                    .column("CellVelocity_x", soci::dt_double)
                    .column("CellVelocity_y", soci::dt_double);
                systemsLogger->info("Table 'entity_map_position' created.");
            } catch (const std::exception& e) {
                systemsLogger->error("Error creating table 'entity_map_position': {}", e.what());
            }
        })
        .set_doc_brief("System to create the position logger table on startup");
        
    ecs.system<const Coordinates::Grid, const Coordinates::Cell, const Coordinates::CellVelocity, Database::Connection>("LogEntityMapPosition")
        .term_at(0).in()
        .term_at(1).in()
        .term_at(2).in()
        .tick_source(Ticks::tick_render)  //TODO: This should probably be a different tick
        .run([](flecs::iter& it) {
            while (it.next()) {
                auto grids = it.field<const Coordinates::Grid>(0);
                auto cells = it.field<const Coordinates::Cell>(1);
                auto cell_velocities = it.field<const Coordinates::CellVelocity>(2);
                auto db_conn = it.field<Database::Connection>(3);


                // Use a single transaction for all the pawn inserts for efficiency
                //  Note that this system runs per StateUtility Second, so the system runs once per State, not
                //  once
                soci::transaction tr(*(db_conn->sql));
                systemsLogger->trace("Logging Entity Position/Velocity/Attitude for {} entities", it.count());
                for (auto i : it) {
                    double time = static_cast<double>(it.world().get_info()->world_time_total);
                    flecs::entity e = it.entity(i);
                    std::string entity_name = std::string(e.path());

                    // Must convert to double for soci
                    double cell_x = static_cast<double>(cells[i].x);
                    double cell_y = static_cast<double>(cells[i].y);
                    double vel_x = static_cast<double>(cell_velocities[i].x);
                    double vel_y = static_cast<double>(cell_velocities[i].y);
                    int32_t grid_x = grids[i].x;
                    int32_t grid_y = grids[i].y;
                    systemsLogger->trace("Entity: {}, Grid: ({}, {}), Cell: ({}, {}), CellVelocity: ({}, {})", 
                        entity_name, grid_x, grid_y, cell_x, cell_y, vel_x, vel_y);

                    *db_conn->sql << "INSERT INTO entity_map_position (time, entity_name, Cell_x, Cell_y, Grid_x, Grid_y, CellVelocity_x, CellVelocity_y) "
                                    "VALUES (:time, :entity_name, :cell_x, :cell_y, CAST(:grid_x AS INTEGER), CAST(:grid_y AS INTEGER), :vel_x, :vel_y)",
                                    soci::use(time, "time"),
                                    soci::use(entity_name, "entity_name"),
                                    soci::use(cell_x, "cell_x"),
                                    soci::use(cell_y, "cell_y"),
                                    soci::use(grid_x, "grid_x"),
                                    soci::use(grid_y, "grid_y"),
                                    soci::use(vel_x, "vel_x"),
                                    soci::use(vel_y, "vel_y");
                }
                tr.commit();
            }
        })
        .set_doc_brief("Log the map position of each entity");

}

}
