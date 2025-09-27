#include "database/module.hpp"

#include <tracy/Tracy.hpp>

#include "msgLogging/module.hpp"
#include "ticks/module.hpp"
#include "pawn/module.hpp"
#include "statemachine/module.hpp"

#include <soci/sqlite3/soci-sqlite3.h>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <cstdio> // For std::remove
#include <future>

namespace Database {

std::shared_ptr<spdlog::logger> systemsLogger;

/**
 * @brief Writes a memory buffer to a file.
 * This is intended to be run in a background thread.
 */
/*
void save_buffer_to_file(const std::string& destFilename, std::shared_ptr<std::vector<unsigned char>> buffer) {
    ZoneScopedN("save_buffer_to_file");
    try {
        std::ofstream of(destFilename, std::ios::binary);
        of.write(reinterpret_cast<const char*>(buffer->data()), buffer->size());
        systemsLogger->info("Database successfully saved to {}", destFilename);
    } catch (const std::exception& e) {
        systemsLogger->error("Failed to write database to file {}: {}", destFilename, e.what());
    }
}
    */

/**
 * @brief Serializes an in-memory SQLite database into a memory buffer.
 * 
 * @param sourceSession The in-memory SOCI session to serialize.
 * @return A shared_ptr to a vector containing the serialized database.
 */
/*
std::shared_ptr<std::vector<unsigned char>> serialize_database(soci::session& sourceSession) {
    ZoneScopedN("serialize_database");
    soci::sqlite3_session_backend* sourceBackend = static_cast<soci::sqlite3_session_backend*>(sourceSession.get_backend());
    sqlite3* pFrom = sourceBackend->conn_;

    sqlite3_int64 size = 0;
    unsigned char* pData = sqlite3_serialize(pFrom, "main", &size, 0);

    if (!pData) {
        systemsLogger->error("Failed to serialize database.");
        return nullptr;
    }

    auto buffer = std::make_shared<std::vector<unsigned char>>(pData, pData + size);
    sqlite3_free(pData);
    return buffer;
}
*/

systems::systems(flecs::world& ecs) {
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sink);
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
                conn.sql = std::make_unique<soci::session>(soci::sqlite3, db_connection_string);
                systemsLogger->info("In-memory database established.");

                // Enable Write-Ahead Logging.
                *conn.sql << "PRAGMA journal_mode=WAL;";
                systemsLogger->info("SQLite journal_mode set to WAL.");
            } catch (const std::exception& e) {
                systemsLogger->error("Failed to open in-memory database connection: {}", e.what());
                conn.sql = nullptr;
            }
        });

    
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
    
    /*
    ecs.system<Database::Connection>("FlushDatabaseToDisk_Periodic")
        .interval(20.0f)
        .each([](Database::Connection& conn) {
            // Clean up any finished futures
            std::erase_if(conn.backup_futures,
                    [](const std::future<void>& f) {
                        // Check if the future is ready with a zero timeout
                        return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                    });

            systemsLogger->info("Initiating periodic asynchronous database flush...");
            
            // 1. Main thread quickly serializes the in-memory DB to a memory buffer.
            auto db_buffer = serialize_database(*conn.sql);
            if (!db_buffer) {
                return; // Serialization failed
            }

            // 2. Launch a background task to write the buffer to disk.
            conn.backup_futures.push_back(std::async(std::launch::async, 
                save_buffer_to_file, "database_backup.db", db_buffer));
        })
        .set_doc_brief("Periodically flushes the in-memory database to disk asynchronously.");
    */

    // System to flush the in-memory database to disk on application shutdown.
    /*
    ecs.system<Database::Connection>("FlushDatabaseToDisk_OnShutdown")
        .kind(flecs::OnStop)
        .each([](Database::Connection& conn) {
            if (!conn.sql) return;

            // Wait for any outstanding async backups to complete.
            systemsLogger->info("Waiting for periodic backup tasks to finish...");
            for (auto& f : conn.backup_futures) {
                f.wait();
            }
            conn.backup_futures.clear();

            systemsLogger->info("Initiating final database flush on shutdown...");
            auto db_buffer = serialize_database(*conn.sql);
            if (db_buffer) {
                save_buffer_to_file("database_final.db", db_buffer);
            }
        });
    */

    
    ecs.system<const Statemachine::StateUtility, Database::Connection>("LogPawnStateUtility")
        .term_at(0).in()
        .term_at(0).second(flecs::Wildcard)
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
