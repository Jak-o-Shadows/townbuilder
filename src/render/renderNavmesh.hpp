//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
#pragma once

#include <vector>

#include <recastnavigation/DetourNavMesh.h>
#include <recastnavigation/Recast.h>
#include <imgui.h>

#include "pathfinding.hpp"


namespace Render {
namespace Navmesh {

void ImDrawRawTriangles(ImDrawList* dd, const Pathfinding::MapTriangles& triangles);


void ImDrawListTriMesh(ImDrawList* dd, const float* verts, int nverts, const int* tris, const float* normals, int ntris, const unsigned char* flags, const float texScale);
void ImDrawListTriMeshSlope(ImDrawList* dd, const float* verts, int nverts, const int* tris, const float* normals, int ntris, const float walkableSlopeAngle, const float texScale);

void ImDrawListHeightfieldSolid(ImDrawList* dd, const rcHeightfield& hf);
void ImDrawListHeightfieldWalkable(ImDrawList* dd, const rcHeightfield& hf);

void ImDrawListCompactHeightfieldSolid(ImDrawList* dd, const rcCompactHeightfield& chf);
void ImDrawListCompactHeightfieldRegions(ImDrawList* dd, const rcCompactHeightfield& chf);
void ImDrawListCompactHeightfieldDistance(ImDrawList* dd, const rcCompactHeightfield& chf);

void ImDrawListHeightfieldLayer(ImDrawList* dd, const rcHeightfieldLayer& layer, const int idx);
void ImDrawListHeightfieldLayers(ImDrawList* dd, const rcHeightfieldLayerSet& lset);
void ImDrawListHeightfieldLayersRegions(ImDrawList* dd, const rcHeightfieldLayerSet& lset);

void ImDrawListRegionConnections(ImDrawList* dd, const rcContourSet& cset, const float alpha = 1.0f);
void ImDrawListRawContours(ImDrawList* dd, const rcContourSet& cset, const float alpha = 1.0f);
void ImDrawListContours(ImDrawList* dd, const rcContourSet& cset, const float alpha = 1.0f);
void ImDrawListPolyMesh(ImDrawList* dd, const rcPolyMesh& mesh);
void ImDrawListPolyMeshDetail(ImDrawList* dd, const rcPolyMeshDetail& dmesh);

}
}