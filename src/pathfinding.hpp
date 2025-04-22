#pragma once
#include <iostream>
#include <memory>

#include <flecs.h>
#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/Recast.h>
#include <spdlog/spdlog.h>  // Unlike most modules, need this imported to alias it to the recast logger
#include <imgui.h>

namespace Pathfinding {

/// These are just sample areas to use consistent values across the samples.
/// The use should specify these base on his needs.
enum SamplePolyAreas
{
	SAMPLE_POLYAREA_GROUND,
	SAMPLE_POLYAREA_WATER,
	SAMPLE_POLYAREA_ROAD,
	SAMPLE_POLYAREA_DOOR,
	SAMPLE_POLYAREA_GRASS,
	SAMPLE_POLYAREA_JUMP
};
enum SamplePolyFlags
{
	SAMPLE_POLYFLAGS_WALK		= 0x01,		// Ability to walk (ground, grass, road)
	SAMPLE_POLYFLAGS_SWIM		= 0x02,		// Ability to swim (water).
	SAMPLE_POLYFLAGS_DOOR		= 0x04,		// Ability to move through doors.
	SAMPLE_POLYFLAGS_JUMP		= 0x08,		// Ability to jump.
	SAMPLE_POLYFLAGS_DISABLED	= 0x10,		// Disabled polygon
	SAMPLE_POLYFLAGS_ALL		= 0xffff	// All abilities.
};

struct Vertex {
    float x;
    float y;
    float z;
    ImVec2 toImVec2() {
        return ImVec2(x, y);
    }
};

struct Triangle {
    int v1Idx;
    int v2Idx;
    int v3Idx;
};




struct MapTriangles{
    /*Vertices is conceptually a [3xn] array. e.g. [0] is p1.x, [1]=p1.y, [2]=p1.z, [3]=p2.x, etc.*/
    std::vector<Vertex> vertices;
    /*Triangles is a [3xn] array.*/
    std::vector<Triangle> triangles;
    
    inline int triangleIdxGet(int n){
        return n;
    }
    inline int vertexIdxGet(int n){
        return n;
    }
    int ntris(){
        return triangles.size();
    }
    int nverts(){
        return vertices.size();
    }
};


struct NavmeshDebugStuff {
    std::shared_ptr<rcPolyMeshDetail> polyMeshDetail;
};






class MyBuildContext: public rcContext {
public:
    MyBuildContext(std::shared_ptr<spdlog::logger> logger) :
        _logger(logger) {}
    std::shared_ptr<spdlog::logger> _logger;
protected:
void doLog(const rcLogCategory category, const char* msg, const int len) {
    std::string message(msg, len);
    switch (category) {
        case RC_LOG_ERROR:
            _logger->error(message);
            break;
        case RC_LOG_WARNING:
            _logger->warn(message);
            break;
        case RC_LOG_PROGRESS:
            _logger->info(message);
            break;
    }
}
};









struct module {
    module(flecs::world& ecs);
};


}