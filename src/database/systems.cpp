#include "database/module.hpp"
#include "msgLogging/module.hpp"
#include "ticks/module.hpp"
#include "pawn/module.hpp"
#include "statemachine/module.hpp"

#include <soci/sqlite3/soci-sqlite3.h>
#include <string>
#include <vector>
#include <cstdio> // For std::remove
#include <iostream>

namespace Database {

std::shared_ptr<spdlog::logger> systemsLogger;

systems::systems(flecs::world& ecs) {
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Database systems module created");

    // This observer opens the database connection when the Connection component is set.
    ecs.observer<Connection>("Observer_OpenDatabaseConnection")
        .term_at(0).singleton()
        .event(flecs::OnSet)
        .each([](Connection& conn) {
            systemsLogger->trace("Creating database connection");
            // Delete the old database file to start fresh
            const char* db_file = "database.db";
            if (std::remove(db_file) == 0) {
                systemsLogger->info("Removed existing database file '{}' to start fresh.", db_file);
            }

            // Create and assign the new session
            try {
                conn.sql = std::make_unique<soci::session>(soci::sqlite3, db_file);
                systemsLogger->info("Database connection established to {}", db_file);
            } catch (const std::exception& e) {
                systemsLogger->error("Failed to open database connection: {}", e.what());
                conn.sql = nullptr;
            }
        });

    
    // System to create the pawn_state_utility table on startup
    ecs.system<Database::Connection>("CreateTable_PawnStateUtility")
        .term_at(0).singleton()
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
        .term_at(1).singleton()
        .with<Pawn::PawnFSMContainer>()
        .tick_source(Ticks::tick_pawn_behaviour)
        .run([](flecs::iter& it) {
            while (it.next()) {
                auto state_utils = it.field<const Statemachine::StateUtility>(0);
                auto db_conn = it.field<Database::Connection>(1);

                // Use a single transaction for all the pawn inserts for efficiency
                //  Note that this system runs per StateUtility Second, so the system runs once per State, not
                //  once
                soci::transaction tr(*(db_conn->sql));
                systemsLogger->trace("Logging state utilities for {} pawns", it.count());
                for (auto i : it) {
                    double time = static_cast<double>(it.world().get_info()->world_time_total);
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
        
}

}
