#include <tracy/Tracy.hpp>

#include "dis/module.hpp"
#include "msgLogging/module.hpp"

#include <KDIS/DataTypes/EntityType.hpp>
#include <KDIS/DataTypes/Enums/EnumEntityInfoInteraction.hpp>
#include <KDIS/Extras/PDU_Factory.hpp>
#include <KDIS/Extras/PDU_Factory_Filters.hpp>

namespace fdis{

std::shared_ptr<spdlog::logger> systemsLogger;

systems::systems(flecs::world& ecs){
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>().sinks);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Module Created");


   ecs.observer<DisConnection>("Obs_DisConnectionConnect")
        .event(flecs::OnSet)
        .each([](DisConnection& con) {
            ZoneScopedN("Obs_DisConnectionConnect");
            //KDIS::UTILS::FactoryFilterExerciseID pduFilter(con.exerciseID);
            //std::cout << "filter created" << std::endl;
            //KDIS::UTILS::PDU_Factory pduFactory;
            //std::cout << "pdu factory created" << std::endl;
            //pduFactory.AddFilter(&pduFilter);
            //std::cout << "added filter" << std::endl;
            //con.con = std::unique_ptr<KDIS::NETWORK::Connection>(new KDIS::NETWORK::Connection(con.sendAddress, con.port));
            //std::cout << "connection created" << std::endl;
            systemsLogger->info("Dis Connected");
    });

};

}