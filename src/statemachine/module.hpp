#pragma once

#include <flecs.h>
#include <Eigen/Dense>

#include <string>


namespace Statemachine {

struct CurveFile {
    std::string filename;
};

struct Curve{
    // Eigen 1D linear interpolator
    std::vector<Eigen::Matrix<float, Eigen::Dynamic, 2>> curves; // x,y pairs sorted on x
    float interpolate(const int curveIdx, const float x) const{
        const Eigen::Matrix<float, Eigen::Dynamic, 2>& points = curves[curveIdx];
        if (x <= points(0,0)) {
            return points(0,1);
        }
        if (x >= points(points.rows()-1,0)) {
            return points(points.rows()-1,1);
        }
        for (int i=1; i<points.rows(); i++) {
            if (x < points(i,0)) {
                // Interpolate between i-1 and i
                float t = (x - points(i-1,0)) / (points(i,0) - points(i-1,0));
                return points(i-1,1) + t * (points(i,1) - points(i-1,1));
            }
        }
    };
};


struct Context {
	flecs::id_t id;
	flecs::world& ecs;
};

struct StateTiming{
	float timeInState_s = 0;
	float culmulativeTimeInState_s = 0;
};



// From https://stackoverflow.com/questions/1055452/c-get-name-of-type-in-template
namespace loggingImpl
{
    template <typename T>
    [[nodiscard]] constexpr std::string_view RawTypeName()
    {
        #ifndef _MSC_VER
        return __PRETTY_FUNCTION__;
        #else
        return __FUNCSIG__;
        #endif
    }

    struct TypeNameFormat
    {
        std::size_t junk_leading = 0;
        std::size_t junk_total = 0;
    };

    constexpr TypeNameFormat type_name_format = []{
        TypeNameFormat ret;
        std::string_view sample = RawTypeName<int>();
        ret.junk_leading = sample.find("int");
        ret.junk_total = sample.size() - 3;
        return ret;
    }();
    static_assert(type_name_format.junk_leading != std::size_t(-1), "Unable to determine the type name format on this compiler.");

    template <typename T>
    static constexpr auto type_name_storage = []{
        std::array<char, RawTypeName<T>().size() - type_name_format.junk_total + 1> ret{};
        std::copy_n(RawTypeName<T>().data() + type_name_format.junk_leading, ret.size() - 1, ret.data());
        return ret;
    }();
}

template <typename T>
[[nodiscard]] constexpr std::string_view TypeName()
{
    return {loggingImpl::type_name_storage<T>.data(), loggingImpl::type_name_storage<T>.size() - 1};
}

template <typename T>
[[nodiscard]] constexpr const char *TypeNameCstr()
{
    return loggingImpl::type_name_storage<T>.data();
}






struct components {
    components(flecs::world& ecs);
};

struct systems {
    systems(flecs::world& ecs);
};


}

