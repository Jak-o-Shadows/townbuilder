#include "statemachine/module.hpp"

#include "msgLogging/module.hpp"

#include <tracy/Tracy.hpp>
#include <rapidcsv.h>

namespace Statemachine {

std::shared_ptr<spdlog::logger> systemsLogger;


systems::systems(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<systems>();
    systemsLogger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    systemsLogger->trace("Module Created");

    
    ecs.observer<CurveFile>("Observer_LoadCurveFile")
        .event(flecs::OnSet)
        .each([](flecs::entity e, const CurveFile& cf){
            ZoneScopedN("Observer_LoadCurveFile");
            systemsLogger->debug("Loading curve file: {}", cf.filename);
            rapidcsv::Document doc(cf.filename, 
                rapidcsv::LabelParams(0, -1),  // Column headers, no row headers
                rapidcsv::SeparatorParams(),  // Required for argument ordering so ConverterParams is there
                rapidcsv::ConverterParams(true));
            
            Curve curveComponent;

            // Each pair of columns is a new curve
            for (size_t colIdx=0; colIdx + 1<doc.GetColumnCount(); colIdx+=2) {
                systemsLogger->debug("Reading curve from columns {} and {}", colIdx, colIdx + 1);
                std::vector<float> x_vec = doc.GetColumn<float>(colIdx);
                std::vector<float> y_vec = doc.GetColumn<float>(colIdx + 1);
                if (x_vec.size() != y_vec.size()){
                    systemsLogger->error("Curve file {} columns {} and {} have different number of entries: {} and {}", cf.filename, colIdx, colIdx+1, x_vec.size(), y_vec.size());
                }

                Eigen::Matrix<float, 2, Eigen::Dynamic> curve_matrix(2, x_vec.size());
                curve_matrix.row(0) = Eigen::Map<Eigen::VectorXf>(x_vec.data(), x_vec.size());
                curve_matrix.row(1) = Eigen::Map<Eigen::VectorXf>(y_vec.data(), y_vec.size());

                curveComponent.curves.push_back(std::move(curve_matrix));
                systemsLogger->trace("Loaded curve with {} points", x_vec.size());
            }
            systemsLogger->trace("Loaded {} curves from file {} into comopnent", curveComponent.curves.size(), cf.filename);
            e.set<Curve>(std::move(curveComponent));

        });

};


}