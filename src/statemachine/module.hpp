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



struct components {
    components(flecs::world& ecs);
};

struct systems {
    systems(flecs::world& ecs);
};


}

