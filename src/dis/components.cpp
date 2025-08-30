#include <tracy/Tracy.hpp>

#include "dis/module.hpp"
#include "msgLogging/module.hpp"

#include <KDIS/DataTypes/EntityType.hpp>
#include <KDIS/DataTypes/Enums/EnumEntityInfoInteraction.hpp>
#include <KDIS/Extras/PDU_Factory.hpp>
#include <KDIS/Extras/PDU_Factory_Filters.hpp>

namespace fdis{

std::shared_ptr<spdlog::logger> logger;

module::module(flecs::world& ecs){
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<module>();
    logger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    logger->trace("Module Created");

    ecs.component<DisConnection>()
        .member<KDIS::KString>("sendAddress")
        .member<KDIS::KUINT32>("port")
        .member<KDIS::KUINT8>("exerciseID");


    // Register the kdis things as components
    /*
    ecs.component<KDIS::DATA_TYPE::EntityType>()
    .opaque(ecs.component()
        .member<KDIS::DATA_TYPE::ENUMS::EntityKind>("EntityKind")
        .member<KDIS::DATA_TYPE::ENUMS::EntityDomain>("Domain")
        .member<KDIS::DATA_TYPE::ENUMS::Country>("Country")
        .member<KDIS::KUINT8>("Category")
        .member<KDIS::KUINT8>("SubCategory")
        .member<KDIS::KUINT8>("Specific")
        .member<KDIS::KUINT8>("Extra"))
    // As protected members, need to serialise them more
    .serialize([](const flecs::serializer *s, const KDIS::DATA_TYPE::EntityType *data){
        s->member("EntityKind");
        s->value(data->GetEntityKind());
        s->member("Domain");
        s->value(data->GetDomain());
        s->member("Country");
        s->value(data->GetCountry());
        s->member("Category");
        s->value(data->GetCategory());
        s->member("SubCategory");
        s->value(data->GetSubCategory());
        s->member("Specific");
        s->value(data->GetSpecific());
        s->member("Extra");
        s->value(data->GetExtra());
    })
    .ensure_member([](KDIS::DATA_TYPE::EntityType* data, const char *member) -> void* {
        if (!strcmp(member, "Domain")) return &data->SetDomain();
        if (!strcmp(member, "Country")) return &data->SetCountry();
        if (!strcmp(member, "Category")) return &data->SetCategory();
        if (!strcmp(member, "SubCategory")) return &data->SetSubCategory();
        if (!strcmp(member, "Specific")) return &data->SetSpecific();
        if (!strcmp(member, "Extra")) return &data->SetExtra();
        return nullptr;
    });
    */



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
            logger->info("Dis Connected");
    });








    }
}