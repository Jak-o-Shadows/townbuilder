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

namespace Render {
namespace Navmesh {

void ImDrawListTriMesh(struct ImDrawList* dd, const float* verts, int nverts, const int* tris, const float* normals, int ntris, const unsigned char* flags, const float texScale);
void ImDrawListTriMeshSlope(struct ImDrawList* dd, const float* verts, int nverts, const int* tris, const float* normals, int ntris, const float walkableSlopeAngle, const float texScale);

void ImDrawListHeightfieldSolid(struct ImDrawList* dd, const struct rcHeightfield& hf);
void ImDrawListHeightfieldWalkable(struct ImDrawList* dd, const struct rcHeightfield& hf);

void ImDrawListCompactHeightfieldSolid(struct ImDrawList* dd, const struct rcCompactHeightfield& chf);
void ImDrawListCompactHeightfieldRegions(struct ImDrawList* dd, const struct rcCompactHeightfield& chf);
void ImDrawListCompactHeightfieldDistance(struct ImDrawList* dd, const struct rcCompactHeightfield& chf);

void ImDrawListHeightfieldLayer(ImDrawList* dd, const struct rcHeightfieldLayer& layer, const int idx);
void ImDrawListHeightfieldLayers(ImDrawList* dd, const struct rcHeightfieldLayerSet& lset);
void ImDrawListHeightfieldLayersRegions(ImDrawList* dd, const struct rcHeightfieldLayerSet& lset);

void ImDrawListRegionConnections(struct ImDrawList* dd, const struct rcContourSet& cset, const float alpha = 1.0f);
void ImDrawListRawContours(struct ImDrawList* dd, const struct rcContourSet& cset, const float alpha = 1.0f);
void ImDrawListContours(struct ImDrawList* dd, const struct rcContourSet& cset, const float alpha = 1.0f);
void ImDrawListPolyMesh(struct ImDrawList* dd, const struct rcPolyMesh& mesh);
void ImDrawListPolyMeshDetail(struct ImDrawList* dd, const struct rcPolyMeshDetail& dmesh);

}
}