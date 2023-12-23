//#define TRACY_ON_DEMAND



#include "tracy_zones.hpp"

#include <flecs.h>
#include <tracy/Tracy.hpp>
#include <perturb/perturb.hpp>



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


#include <cassert>
#include <iostream>
#include <vector>
#include <random>


struct TLE {
    std::string line1;
    std::string line2;
};

struct PerturbSatellite {
    std::unique_ptr<perturb::Satellite> sat;
};




int main(int, char *[]) {

    flecs::world ecs;
    ecs.set<flecs::Rest>({});
    ecs.import<flecs::monitor>(); // Enable statistics in explorer


    perturb::StateVector stateVector;

    // ISS
    ecs.entity("ISS")
        .set<TLE>({"1 25544U 98067A   22071.78032407  .00021395  00000-0  39008-3 0  9996",
                   "2 25544  51.6424  94.0370 0004047 256.5103  89.8846 15.49386383330227"})
        .set<perturb::StateVector>(stateVector);
    // ON 23 DEC 2023
    // SKYKRAFT-3D 
    ecs.entity("SKYKRAFT-3D ")
        .set<TLE>({"1 57956U 23084BZ  23356.86776708  .00012856  00000+0  66259-3 0  9999",
                   "2 57956  97.5277 111.2278 0010377 268.2908  91.7136 15.16462747 15664"})
        .set<perturb::StateVector>(stateVector);
    // SKYKRAFT-3A
    ecs.entity("SKYKRAFT-3A")
        .set<TLE>({"1 56981U 23084BB  23357.07279421  .00027189  00000+0  12531-2 0  9998",
                   "2 56981  97.5256 111.8495 0009723 263.5412  96.4715 15.20201500 29276"})
        .set<perturb::StateVector>(stateVector);
    // SKYKRAFT-3C
    ecs.entity("SKYKRAFT-3C")
        .set<TLE>({"1 56228U 23084BM  23357.13436977  .00022918  00000+0  10920-2 0  9995",
                   "2 56228  97.5265 111.7796 0009960 264.6682  95.3416 15.19083501 29270"})
        .set<perturb::StateVector>(stateVector);
    // SKYKRAFT-3B
    ecs.entity("SKYKRAFT-3B")
        .set<TLE>({"1 56227U 23084BL  23357.12105524  .00030957  00000+0  14074-2 0  9990",
                   "2 56227  97.5256 111.9421 0009668 263.6040  96.4094 15.20644082 29282"})
        .set<perturb::StateVector>(stateVector);
    // SKYKRAFT-1C
    ecs.entity("SKYKRAFT-1C")
        .set<TLE>({"1 55061U 23001BE  23356.89382718  .00004494  00000+0  24077-3 0  9991",
                   "2 55061  97.4551  52.7786 0014276  83.3557 276.9300 15.15418361 53613"})
        .set<perturb::StateVector>(stateVector);
    // SKYKRAFT-1B
    ecs.entity("SKYKRAFT-1B")
        .set<TLE>({"1 55058U 23001BB  23356.74933113  .00014355  00000+0  66599-3 0  9994",
                   "2 55058  97.4545  53.7039 0013248  76.6152 283.6559 15.20071284 53669"})
        .set<perturb::StateVector>(stateVector);
    // SKYKRAFT-1A
    ecs.entity("SKYKRAFT-1A")
        .set<TLE>({"1 55054U 23001AX  23356.91060384  .00006089  00000+0  31569-3 0  9994",
                   "2 55054  97.4545  53.0267 0014126  79.2867 280.9956 15.16442057 53632"})
        .set<perturb::StateVector>(stateVector);
    // SKYKRAFT-1D
    ecs.entity("SKYKRAFT-1D")
        .set<TLE>({"1 55052U 23001AV  23356.62070823  .00019345  00000+0  84824-3 0  9990",
                   "2 55052  97.4543  54.0235 0012700  73.9741 286.2893 15.21950491 53677"})
        .set<perturb::StateVector>(stateVector);
    // SKYKRAFT-1
    ecs.entity("SKYKRAFT-1")
        .set<TLE>({"1 55041U 23001AJ  23357.17107852  .00011354  00000+0  54839-3 0  9997",
                   "2 55041  97.4550  53.7709 0013514  75.8584 284.4151 15.18730549 53702"})
        .set<perturb::StateVector>(stateVector);
    // CENTAURI-4 (TYVAK-0211)
    ecs.entity("CENTAURI-4 (TYVAK-0211)")
        .set<TLE>({"1 48898U 21059V   23356.93985918  .00064926  00000+0  11355-2 0  9998",
                   "2 48898  97.6043 141.7934 0005104 311.2054  48.8756 15.50408230138494"})
        .set<perturb::StateVector>(stateVector);
    // TYVAK-0173
    ecs.entity("TYVAK-0173")
        .set<TLE>({"1 48892U 21059P   23356.79518159  .00004140  00000+0  60641-3 0  9990",
                   "2 48892  96.0152 286.8262 0009098 205.8911 154.1850 14.75928033134129"})
        .set<perturb::StateVector>(stateVector);
    // M2-B
    ecs.entity("M2-B")
        .set<TLE>({"1 47973U 21023J   23356.92981442  .00019685  00000+0  94707-3 0  9994",
                   "2 47973  44.9959 174.8082 0010909 184.6256 175.4524 15.18496097125896"})
        .set<perturb::StateVector>(stateVector);
    // MYRIOTA 7 (tYVAK-0152)
    ecs.entity("MYRIOTA 7 (tYVAK-0152)")
        .set<TLE>({"1 47968U 21023D   23356.84072537  .00027576  00000+0  11631-2 0  9997",
                   "2 47968  44.9982 164.5576 0010354 201.3542 158.6909 15.22840294151884"})
        .set<perturb::StateVector>(stateVector);
    // CENTAURI-3 (TYVAK-0210)
    ecs.entity("CENTAURI-3 (TYVAK-0210)")
        .set<TLE>({"1 47966U 21023B   23356.76190689  .00046221  00000+0  14262-2 0  9996",
                   "2 47966  45.0066 150.6388 0007838 258.5402 101.4603 15.32892371152046"})
        .set<perturb::StateVector>(stateVector);
    // TYVAK-0171
    ecs.entity("TYVAK-0171")
        .set<TLE>({"1 46295U 20061Z   23356.74483493  .00233594  00000+0  26031-2 0  9995",
                   "2 46295  97.3816  70.5637 0002870  10.1502 349.9813 15.62056039182966"})
        .set<perturb::StateVector>(stateVector);
    // M2 PATHFINDER
    ecs.entity("M2 PATHFINDER")
        .set<TLE>({"1 45727U 20037E   23356.03892843  .00004707  00000+0  42925-3 0  9995",
                   "2 45727  97.7637 168.7438 0011772   6.5909 353.5465 14.95538946189306"})
        .set<perturb::StateVector>(stateVector);
    // CENTAURI-1
    ecs.entity("CENTAURI-1")
        .set<TLE>({"1 43809U 18099BD  23356.90837044  .00006962  00000+0  54289-3 0  9998",
                   "2 43809  97.5596  54.1174 0017286 211.7319 148.2866 15.01365145275669"})
        .set<perturb::StateVector>(stateVector);
    // PROXIMA II
    ecs.entity("PROXIMA II")
        .set<TLE>({"1 43696U 18088G   23356.85106320  .00071699  00000+0  10975-2 0  9992",
                   "2 43696  85.0175 298.7153 0008387 165.0370 195.1139 15.54061398284903"})
        .set<perturb::StateVector>(stateVector);
    // PROXIMA I
    ecs.entity("PROXIMA I")
        .set<TLE>({"1 43694U 18088E   23356.94221584  .00070090  00000+0  10598-2 0  9995",
                   "2 43694  85.0168 298.5541 0008350 163.6216 196.5315 15.54404331284937"})
        .set<perturb::StateVector>(stateVector);
    // DIAMOND BLUE
    ecs.entity("DIAMOND BLUE")
        .set<TLE>({"1 42786U 17036X   23356.91285080  .00084822  00000+0  10779-2 0  9995",
                   "2 42786  97.1253  29.5290 0006385 115.6260 244.5657 15.59091600362073"})
        .set<perturb::StateVector>(stateVector);
    // DIAMOND GREEN
    ecs.entity("DIAMOND GREEN")
        .set<TLE>({"1 42785U 17036W   23356.81845304  .00104724  00000+0  12187-2 0  9993",
                   "2 42785  97.1215  29.7655 0005883 116.6100 243.5761 15.61341482362088"})
        .set<perturb::StateVector>(stateVector);
    // DIAMOND RED
    ecs.entity("DIAMOND RED")
        .set<TLE>({"1 42783U 17036U   23356.61345504  .00079393  00000+0  10760-2 0  9994",
                   "2 42783  97.1241  28.8304 0005991 121.2299 238.9545 15.57393220361930"})
        .set<perturb::StateVector>(stateVector);
    // SKY MUSTER 2 (NBN1B)
    ecs.entity("SKY MUSTER 2 (NBN1B)")
        .set<TLE>({"1 41794U 16060B   23357.06507669 -.00000233  00000+0  00000+0 0  9994",
                   "2 41794   0.0178 330.4309 0001746 291.1559 357.9939  1.00268939 26477"})
        .set<perturb::StateVector>(stateVector);
    // SKY MUSTER 1 (NBN1A)
    ecs.entity("SKY MUSTER 1 (NBN1A)")
        .set<TLE>({"1 40940U 15054A   23357.07063404 -.00000267  00000+0  00000+0 0  9990",
                   "2 40940   0.0167 323.9866 0001986 298.6923 354.3533  1.00269848 30156"})
        .set<perturb::StateVector>(stateVector);
    // OPTUS 10
    ecs.entity("OPTUS 10")
        .set<TLE>({"1 40146U 14054A   23357.05952209 -.00000136  00000+0  00000+0 0  9990",
                   "2 40146   0.0692  89.1453 0003702 160.0103  19.6158  1.00269102 34027"})
        .set<perturb::StateVector>(stateVector);
    // OPTUS D3
    ecs.entity("OPTUS D3")
        .set<TLE>({"1 35756U 09044B   23357.12985595 -.00000136  00000+0  00000+0 0  9996",
                   "2 35756   0.0348  31.3836 0003746 256.1582   6.6184  1.00271290 52538"})
        .set<perturb::StateVector>(stateVector);
    // OPTUS D2
    ecs.entity("OPTUS D2")
        .set<TLE>({"1 32252U 07044A   23356.67688302 -.00000100  00000+0  00000+0 0  9998",
                   "2 32252   0.0307 282.6599 0003636 347.3741 224.6043  1.00269992 59478"})
        .set<perturb::StateVector>(stateVector);
    // OPTUS D1
    ecs.entity("OPTUS D1")
        .set<TLE>({"1 29495U 06043B   23356.92531997 -.00000172  00000+0  00000+0 0  9995",
                   "2 29495   0.6858  90.8488 0003951 181.4733 304.0009  1.00271857 63028"})
        .set<perturb::StateVector>(stateVector);
    // OPTUS C1
    ecs.entity("OPTUS C1")
        .set<TLE>({"1 27831U 03028B   23356.62844338 -.00000143  00000+0  00000+0 0  9998",
                   "2 27831   4.5955  80.2337 0006142 191.6005 200.9982  1.00272826 43969"})
        .set<perturb::StateVector>(stateVector);
    // SPIRIT
    ecs.entity("SPIRIT")
        .set<TLE>({"1 58468U 23185G   23357.11756912  .00006799  00000+0  34809-3 0  9999",
                   "2 58468  97.4526  61.3089 0013947 175.3806 184.7558 15.16847854  3243"})
        .set<perturb::StateVector>(stateVector);















    std::string ISS_TLE_1 = "1 25544U 98067A   22071.78032407  .00021395  00000-0  39008-3 0  9996";
    std::string ISS_TLE_2 = "2 25544  51.6424  94.0370 0004047 256.5103  89.8846 15.49386383330227";

    // Create and initialize a satellite object from the TLE
    auto sat = perturb::Satellite::from_tle(ISS_TLE_1, ISS_TLE_2);
    assert(sat.last_error() == perturb::Sgp4Error::NONE);
    assert(sat.epoch().to_datetime().day == 12);

    // Let's see what the ISS is doing on Pi Day
    const auto t = perturb::JulianDate(perturb::DateTime { 2022, 3, 14, 1, 59, 26.535 });
    const double delta_days = t - sat.epoch();
    assert(1 < delta_days && delta_days < 3);  // It's been ~2 days since the epoch

    // Calculate the position and velocity at the chosen time
    perturb::StateVector sv;
    const auto err = sat.propagate(t, sv);
    assert(err == perturb::gp4Error::NONE);
    const auto &pos = sv.position, &vel = sv.velocity;
    perturb::Vec3 p;
    p = sv.position;

    // Conclusion: The ISS is going pretty fast (~8 km/s)
    std::cout << "Position [km]: { " << pos[0] << ", " << pos[1] << ", " << pos[2] << " }\n";
    std::cout << "Velocity [km/s]: { " << vel[0] << ", " << vel[1] << ", " << vel[2] << " }\n";














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







    ecs.system<TLE>(ts_createPerturbSatellite)
        .kind(flecs::OnStart)
        .each([](flecs::entity e, TLE& tle){
        ZoneScopedN(ts_createPerturbSatellite);

        // Can't figure out how to use the alternate constructor, so do it by hand;
        perturb::TwoLineElement tleStruct;
        tleStruct.parse(tle.line1, tle.line2);

        e.set<PerturbSatellite>({std::unique_ptr<perturb::Satellite>(new perturb::Satellite(tleStruct))});

        });




    ecs.system<TLE, perturb::StateVector, PerturbSatellite>(ts_propagatePerturbSatellite)
        .tick_source(tick_100_Hz)
        .multi_threaded()
        .each([](flecs::entity e, TLE& tle, perturb::StateVector& sv, PerturbSatellite& sat){
        ZoneScopedN(ts_propagatePerturbSatellite);

        // Let's see what the ISS is doing on Pi Day
        const auto t = perturb::JulianDate(perturb::DateTime { 2022, 3, 14, 1, 59, 26.535 });
        const double delta_days = t - sat.sat->epoch();
        assert(1 < delta_days && delta_days < 3);  // It's been ~2 days since the epoch

        // Calculate the position and velocity at the chosen time
        const auto err = sat.sat->propagate(t, sv);
        assert(err == perturb::gp4Error::NONE);
        //const auto &pos = sv.position, &vel = sv.velocity;
        perturb::Vec3& pos = sv.position;
        perturb::Vec3& vel = sv.velocity;

        // Conclusion: The ISS is going pretty fast (~8 km/s)
        //std::cout << "Position [km]: { " << pos[0] << ", " << pos[1] << ", " << pos[2] << " }" << std::endl;
        //std::cout << "Velocity [km/s]: { " << vel[0] << ", " << vel[1] << ", " << vel[2] << " }" << std::endl;

        });






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