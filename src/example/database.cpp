#include "example/module.hpp"

#include "example/statemachine.hpp"

#include "msgLogging/module.hpp"
#include "database/module.hpp"
#include "ticks/module.hpp"

#include <spdlog/spdlog.h>

namespace Example {

std::shared_ptr<spdlog::logger> databaseLogger;

database::database(flecs::world& ecs) {
    flecs::entity m = ecs.module<database>();
    databaseLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    databaseLogger->trace("Example database module created");

    ecs.import<Example::statemachine>();
    ecs.import<Ticks::module>();
    ecs.import<Database::components>();

    // Log the statemachine currently active state
    // System to create the pawn current table on startup
    ecs.system<Database::Connection>("CreateTable_ExampleActiveStates")
        .kind(flecs::OnStart)
        .each([](Database::Connection& conn) {
            ZoneScopedN("CreateTable_ExampleActiveStates");
            databaseLogger->trace("Creating table 'example_active_states'");
            try {
                soci::session sql(*conn.pool);
                sql.create_table("example_active_states")
                    .column("time", soci::dt_double)
                    .column("entity_name", soci::dt_string)
                    .column("State1", soci::dt_integer)
                    .column("State2", soci::dt_integer)
                    .column("State3", soci::dt_integer)
                    .column("State4", soci::dt_integer)
                    ;
                databaseLogger->info("Table 'example_active_states' created.");
            } catch (const std::exception& e) {
                databaseLogger->error("Error creating table 'example_active_states': {}", e.what());
            }
        });    

    ecs.system<Example::FSMContainer, Database::Connection>("LogExampleStateActive")
        .tick_source(Ticks::tick_pawn_behaviour)  // TODO: Swap to a mroe generic tick source
        .run([](flecs::iter& it) {
            ZoneScopedN("LogExampleStateActive");
            databaseLogger->trace("Logging active states for {} pawns", it.count());
            while (it.next()) {
                auto fsmc = it.field<Example::FSMContainer>(0);
                auto db_conn = it.field<Database::Connection>(1);

                soci::session sql(*db_conn->pool);

                // Use a single transaction for all the entity inserts for efficiency
                soci::transaction tr(sql);
                double time = static_cast<double>(it.world().get_info()->world_time_total);
                for (auto i : it) {
                    flecs::entity e = it.entity(i);
                    std::string entity_name = std::string(e.path());

                    // SOCI can't take the values directly inline, so must assign to variables first
                    int state1 = static_cast<int>(fsmc[i].machine->isActive<Example::State1>());
                    int state2 = static_cast<int>(fsmc[i].machine->isActive<Example::State2>());
                    int state3 = static_cast<int>(fsmc[i].machine->isActive<Example::State3>());
                    int state4 = static_cast<int>(fsmc[i].machine->isActive<Example::State4>());

                    sql << "INSERT INTO example_active_states (time, entity_name, State1, State2, State3, State4) "
                                    "VALUES (:time, :entity_name, :state1, :state2, :state3, :state4)",
                                    soci::use(time, "time"),
                                    soci::use(entity_name, "entity_name"),
                                    soci::use(state1, "state1"),
                                    soci::use(state2, "state2"),
                                    soci::use(state3, "state3"),
                                    soci::use(state4, "state4");
                }
                tr.commit();
            }
        });

}

}