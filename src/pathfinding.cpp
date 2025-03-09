#include <tracy/Tracy.hpp>

#include "pathfinding.hpp"
#include "msgLogging.hpp"

#include <recastnavigation/Recast.h>

#include <iostream>

#include "gridMap.hpp"


namespace Pathfinding{

   

std::shared_ptr<spdlog::logger> logger;



module::module(flecs::world& ecs){
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<module>();
    logger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::trace});
    logger->trace("Module Created");







	//ecs.component<NavmeshDebugStuff>().add(flecs::Sparse);
	//logger->trace("Components Registered");




    // Create navmesh
    flecs::entity mapEntity = Map::mapEntity;  // TODO: Shouldn't be doing it from here - get it in a safer way
    const Map::Grid* map = mapEntity.get<Map::Grid>();
    int width = map->m_width;
    int height = map->m_height;
    float bmin[] = {0, 0, 0};
    float bmax[] = {static_cast<float>(width)+1, static_cast<float>(height)+1, 0};
    const int cellSize = 1;
    const int cellHeight = 1;

    float walkableSlopeAngle = 70;  // degrees
    float walkableClimb = 100;
    float walkableHeight = 1;
    float walkableRadius = 1;

    bool m_keepInterResults = false;
    bool m_filterLowHangingObstacles = false;
    bool m_filterLedgeSpans = false;
    bool m_filterWalkableLowHeightSpans = false;

    // TODO: Determine sensible parameters
    float minRegionArea = 1.0f;  // Minimum region size in voxel units. Smaller regions will be merged or removed.
    float mergeRegionArea = 20.0f;  // Any regions with a span count smaller than this value will, if possible, be merged with larger regions.
    float maxSimplificationError = 1.3f;  // The maximum distance a simplification vertex can deviate from the original contour.
    float maxEdgeLen = 12.0f;  // The maximum allowed length for contour edges along the border of the mesh.
    int maxVertsPerPoly = 6;  // The maximum number of vertices allowed for polygons generated during the contour to polygon conversion process.
    float detailSampleDist = 6.0f;  // Sets the sampling distance to use when generating the detail mesh.
    float detailSampleMaxError = 1.0f;  // The maximum distance the detail mesh surface should deviate from heightfield data.


	MyBuildContext* ctx = new MyBuildContext(logger);
    ctx->enableLog(true);

    rcHeightfield* m_solid = rcAllocHeightfield();
	if (!m_solid)
	{
	    ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
	}
	if (!rcCreateHeightfield(ctx, *m_solid, width, height, bmin, bmax, cellSize, cellHeight))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
	}

    std::vector<float> vertices;
    std::vector<int> triangles;

    // Generate vertices
    for (int row=0;row<=height;row++) {
        for (int col=0;col<=width;col++) {
            vertices.push_back(static_cast<float>(col)); // x
            vertices.push_back(static_cast<float>(row)); // y
            vertices.push_back(0.0f);                 // z (assuming z = 0 for a 2D grid)
        }
    }
    // Generate triangles
    for (int row=0;row<height;row++) {
        for (int col=0;col<width;col++) {
            int topLeft = row * (width + 1) * 3 + col * 3;
            int topRight = topLeft + 3;
            int bottomLeft = topLeft + (width + 1) * 3;
            int bottomRight = bottomLeft + 3;

            // Triangle 1
            triangles.push_back(topLeft / 3);
            triangles.push_back(topRight / 3);
            triangles.push_back(bottomRight / 3);
            // Triangle 2
            triangles.push_back(topLeft / 3);
            triangles.push_back(bottomRight / 3);
            triangles.push_back(bottomLeft / 3);
        }
    }
    int nverts = vertices.size()/3;
    int ntris = triangles.size()/3;
    std::cout << "NumVerts: " << nverts << " NumTris: " << ntris << std::endl;


	unsigned char* m_triareas = new unsigned char[ntris];
	if (!m_triareas)
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'm_triareas' (%d).", ntris);
	}
    std::cout << "allocated m_triareas" << std::endl;
    
	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	memset(m_triareas, 0, ntris*sizeof(unsigned char));
    std::cout << "Initialised m_triareas to zero" << std::endl;
	rcMarkWalkableTriangles(ctx, walkableSlopeAngle,
        vertices.data(), nverts,
        triangles.data(), ntris, m_triareas);
    std::cout << "Marked Walkable Triangles" << std::endl;
	if (!rcRasterizeTriangles(ctx, vertices.data(), nverts, triangles.data(), m_triareas, ntris, *m_solid, walkableClimb))
	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not rasterize triangles.");
	}
	if (!m_keepInterResults)
	{
		delete [] m_triareas;
		m_triareas = nullptr;
	}
    std::cout << "walkabletriangles " << std::endl;
    
    // Step 3. Filter walkable surfaces.
	
	// Once all geometry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	if (m_filterLowHangingObstacles) {
		rcFilterLowHangingWalkableObstacles(ctx, walkableClimb, *m_solid);
    }
	if (m_filterLedgeSpans) {
		rcFilterLedgeSpans(ctx, walkableHeight, walkableClimb, *m_solid);
    }
	if (m_filterWalkableLowHeightSpans) {
		rcFilterWalkableLowHeightSpans(ctx, walkableHeight, *m_solid);
    }
    std::cout << "walkable surfaces filtered" << std::endl;

    //
	// Step 4. Partition walkable surface to simple regions.
	//

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	rcCompactHeightfield* m_chf = rcAllocCompactHeightfield();
	if (!m_chf)	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
	}
	if (!rcBuildCompactHeightfield(ctx, walkableHeight, walkableClimb, *m_solid, *m_chf)) {
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
	}
	
	if (!m_keepInterResults) {
		rcFreeHeightField(m_solid);
		m_solid = 0;
	}
		
	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(ctx, walkableRadius, *m_chf)) {
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
	}
    

	// (Optional) Mark areas.
    // TODO: Put thsi back in. It requires more files
    /*
	const ConvexVolume* vols = m_geom->getConvexVolumes();
	for (int i  = 0; i < m_geom->getConvexVolumeCount(); ++i) {
		rcMarkConvexPolyArea(ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);
    }
    */


	// Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
	// There are 3 partitioning methods, each with some pros and cons:
	// 1) Watershed partitioning
	//   - the classic Recast partitioning
	//   - creates the nicest tessellation
	//   - usually slowest
	//   - partitions the heightfield into nice regions without holes or overlaps
	//   - the are some corner cases where this method creates produces holes and overlaps
	//      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
	//      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
	//   * generally the best choice if you precompute the navmesh, use this if you have large open areas
    // Prepare for region partitioning, by calculating distance field along the walkable surface.
    if (!rcBuildDistanceField(ctx, *m_chf))
    {
        ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
    }
    
    // Partition the walkable surface into simple regions without holes.
    if (!rcBuildRegions(ctx, *m_chf, 0, minRegionArea, mergeRegionArea))
    {
        ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
    }

    //
	// Step 5. Trace and simplify region contours.
	//
	// Create contours.
	rcContourSet* m_cset = rcAllocContourSet();
	if (!m_cset) {
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
	}
	if (!rcBuildContours(ctx, *m_chf, maxSimplificationError, maxEdgeLen, *m_cset))	{
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
	}
	
	//
	// Step 6. Build polygons mesh from contours.
	//
	
	// Build polygon navmesh from the contours.
	rcPolyMesh* m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh) {
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
	}
	if (!rcBuildPolyMesh(ctx, *m_cset, maxVertsPerPoly, *m_pmesh)) {
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
	}

    //
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	std::shared_ptr<rcPolyMeshDetail> m_dmesh = std::make_shared<rcPolyMeshDetail>();
	if (!m_dmesh) {
		ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'.");
	}
	if (!rcBuildPolyMeshDetail(ctx, *m_pmesh, *m_chf, detailSampleDist, detailSampleMaxError, *m_dmesh)) {
		ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
	}
	if (!m_keepInterResults) {
		rcFreeCompactHeightfield(m_chf);
		m_chf = 0;
		rcFreeContourSet(m_cset);
		m_cset = 0;
	}



	// Add NavmeshDebugStuff component to the mapEntity
	
	NavmeshDebugStuff debugStuff;
	
	//debugStuff.navMesh = std::make_shared<dtNavMesh>();
	//debugStuff.navQuery = std::make_shared<dtNavMeshQuery>();
	//debugStuff.polyMesh = std::shared_ptr<rcPolyMesh>(m_pmesh, rcFreePolyMesh);
	debugStuff.polyMeshDetail = m_dmesh;

	//mapEntity.set<NavmeshDebugStuff>(debugStuff);
	logger->trace("NavmeshDebugStuff component added to mapEntity");
	


    }
}