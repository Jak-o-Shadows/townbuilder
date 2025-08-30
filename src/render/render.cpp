#include "render.hpp"

#include <tracy/Tracy.hpp>
#include <spdlog/spdlog.h>

#include "flecs_components_transform.h"
#include "flecs_components_graphics.h"
#include "flecs_components_geometry.h"
#include "flecs_components_physics.h"
#include "flecs_components_gui.h"
//#include "flecs_components_input.h"
#include "flecs_systems_transform.h"
//#include "flecs_systems_physics.h"

#include "imgui.h"
#include "../bindings/imgui_impl_glfw.h"
#include "../bindings/imgui_impl_opengl3.h"



#include "componentsPawn.hpp"
#include "componentsBuilding.hpp"
#include "componentsMap.hpp"
#include "ticks.hpp"
#include "pathfinding.hpp"
#include "renderNavmesh.hpp"
#include "coordinates.hpp"

namespace Render{

std::shared_ptr<spdlog::logger> logger;

static void glfw_error_callback(int error, const char *description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void render_conan_logo()
{
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	float sz = 300.0f;
	static ImVec4 col1 = ImVec4(68.0 / 255.0, 83.0 / 255.0, 89.0 / 255.0, 1.0f);
	static ImVec4 col2 = ImVec4(40.0 / 255.0, 60.0 / 255.0, 80.0 / 255.0, 1.0f);
	static ImVec4 col3 = ImVec4(50.0 / 255.0, 65.0 / 255.0, 82.0 / 255.0, 1.0f);
	static ImVec4 col4 = ImVec4(20.0 / 255.0, 40.0 / 255.0, 60.0 / 255.0, 1.0f);
	const ImVec2 p = ImGui::GetCursorScreenPos();
	float x = p.x + 4.0f, y = p.y + 4.0f;
	draw_list->AddQuadFilled(ImVec2(x, y + 0.25 * sz), ImVec2(x + 0.5 * sz, y + 0.5 * sz), ImVec2(x + sz, y + 0.25 * sz), ImVec2(x + 0.5 * sz, y), ImColor(col1));
	draw_list->AddQuadFilled(ImVec2(x, y + 0.25 * sz), ImVec2(x + 0.5 * sz, y + 0.5 * sz), ImVec2(x + 0.5 * sz, y + 1.0 * sz), ImVec2(x, y + 0.75 * sz), ImColor(col2));
	draw_list->AddQuadFilled(ImVec2(x + 0.5 * sz, y + 0.5 * sz), ImVec2(x + sz, y + 0.25 * sz), ImVec2(x + sz, y + 0.75 * sz), ImVec2(x + 0.5 * sz, y + 1.0 * sz), ImColor(col3));
	draw_list->AddLine(ImVec2(x + 0.75 * sz, y + 0.375 * sz), ImVec2(x + 0.75 * sz, y + 0.875 * sz), ImColor(col4));
    draw_list->AddBezierCubic(ImVec2(x + 0.72 * sz, y + 0.24 * sz), ImVec2(x + 0.68 * sz, y + 0.15 * sz), ImVec2(x + 0.48 * sz, y + 0.13 * sz), ImVec2(x + 0.39 * sz, y + 0.17 * sz), ImColor(col4), 10, 18);
    draw_list->AddBezierCubic(ImVec2(x + 0.39 * sz, y + 0.17 * sz), ImVec2(x + 0.2 * sz, y + 0.25 * sz), ImVec2(x + 0.3 * sz, y + 0.35 * sz), ImVec2(x + 0.49 * sz, y + 0.38 * sz), ImColor(col4), 10, 18);
}




module::module(flecs::world& ecs) {
    // Register module with world. The module entity will be created with the
    // same hierarchy as the C++ namespaces (e.g. simple::module)
    flecs::entity m = ecs.module<module>();
    logger = Logging::init_module_logger(m, ecs.get<Logging::LoggerSink>()->sink);
    // Before using logger, must set the level so the observer can handle it
    m.set<Logging::LoggerControls>({spdlog::level::err});
    logger->trace("Module Created");
    std::cout << "Render Module Created" << std::endl;


    ecs.import<Ticks::module>();
    logger->trace("Modules Imported");



    ecs.component<ImColor>()
        .member<float>("r")
        .member<float>("g")
        .member<float>("b")
        .member<float>("a");
    ecs.component<Box>()
        .member<float>("width")
        .member<float>("height")
        .member<float>("depth");
    ecs.component<flecs::components::transform::Position3>()  // TODO: We shouldn't have to be registering this. Am I jsut not importing it?
        .member<float>("x")
        .member<float>("y")
        .member<float>("z");
    logger->trace("Components Registered");


    // Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
        logger->error("Could not init glfwInit");
		for (;;) {};
	}

	// Decide GL+GLSL versions

	// GL 3.0 + GLSL 130
	const char *glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

	// Create window with graphics context
	GLFWwindow *window = glfwCreateWindow(1280, 720, "Dear ImGui - Conan", NULL, NULL);
	if (window == NULL) {
        logger->error("GLFWWindow null");
		for (;;) {};
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	bool err = glewInit() != GLEW_OK;

	if (err)
	{
        logger->error("Failed to initialize OpenGL loader!");
		for (;;) {};
	}

	int screen_width, screen_height;
	glfwGetFramebufferSize(window, &screen_width, &screen_height);
	glViewport(0, 0, screen_width, screen_height);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

    ecs.set<Window>({window});


    ecs.system("glfwWindowPollEvents")
        .kind(flecs::PreFrame)
        .tick_source(Ticks::tick_render)
        .each([](){
        ZoneScopedN("glfwWindowPollEvents");
        logger->trace("glfwWindowPollEvents");

		glfwPollEvents();
		glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
		glClear(GL_COLOR_BUFFER_BIT);
        logger->trace("glClear");

		// feed inputs to dear imgui, start new frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
        logger->trace("ImGui NewFrame");
        });

    ecs.system("ExampleImGuiDrawing")
        .kind(flecs::OnUpdate)
        .tick_source(Ticks::tick_render)
        .each([](){
        ZoneScopedN("ExampleImGuiDrawing");
        logger->trace("ExampleImGuiDrawing");
        ImGui::Begin("Triangle Position/Color");
        static float rotation = 0.0;
        ImGui::SliderFloat("rotation", &rotation, 0, 2 * 3.14159265);
        static float translation[] = {0.0, 0.0};
        ImGui::SliderFloat2("position", translation, -1.0, 1.0);
        static float color[4] = { 1.0f,1.0f,1.0f,1.0f };
        ImGui::End();

        ImGui::Begin("Conan logo");
        render_conan_logo();
        ImGui::End();
        logger->trace("ImGui Render Conan Logo");

        });


    ecs.system<Window>("RenderImGui")
        .term_at(0).singleton()
        .kind(flecs::PostFrame)
        .tick_source(Ticks::tick_render)
        .each([](Window& w){
        ZoneScopedN("RenderImGui");
        logger->trace("RenderImGui");

		// Render dear imgui into screen
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        logger->trace("ImGuiRenderDrawData");

		int display_w, display_h;
		glfwGetFramebufferSize(w.window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glfwSwapBuffers(w.window);
        logger->trace("glfwSwapBuffers");
        });




    
    ecs.system<const Box,
               const flecs::components::transform::Position3,
               const ImColor>("RenderBox")
        .kind(flecs::OnUpdate)
        .tick_source(Ticks::tick_render)
        .term_at(0).in()
        .term_at(1).in()
        .term_at(2).in()
        .run([](flecs::iter& it){
            ZoneScopedN("RenderBox");
            
            ImGui::Begin("Map");
            
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            const ImVec2 screenpos = ImGui::GetCursorScreenPos();

            while (it.next()) {
                auto box = it.field<const Box>(0);
                auto p = it.field<const flecs::components::transform::Position3>(1);
                auto c = it.field<const ImColor>(2);

                for (auto i : it) {
                    flecs::entity e = it.entity(i);
                    ImVec2 corner1 = ImVec2(50+screenpos.x + (p[i].x - box[i].width/2), 50+screenpos.y + (p[i].y - box[i].height/2));
                    ImVec2 corner2 = ImVec2(50+screenpos.x + (p[i].x - box[i].width/2), 50+screenpos.y + (p[i].y + box[i].height/2));
                    ImVec2 corner3 = ImVec2(50+screenpos.x + (p[i].x + box[i].width/2), 50+screenpos.y + (p[i].y + box[i].height/2));
                    ImVec2 corner4 = ImVec2(50+screenpos.x + (p[i].x + box[i].width/2), 50+screenpos.y + (p[i].y - box[i].height/2));
                    draw_list->AddQuadFilled(corner1, corner2, corner3, corner4, c[i]);
                    draw_list->AddQuad(corner1, corner2, corner3, corner4, ImColor(ImVec4(0, 0, 0, 1.0)), 1.0);
                    logger->trace("Rendered {} of: {}, {}, {}, {}", std::string(e.path()), corner1.x, corner1.y, corner3.x, corner3.y);
                }
                
            }

            ImGui::End();
            
    });
    

    ecs.system<const Pathfinding::MapTriangles>("RenderBaseTriangles")
        .kind(flecs::OnUpdate)
        .tick_source(Ticks::tick_render)
        .each([](const Pathfinding::MapTriangles& triangles){
            ZoneScopedN("RenderBaseTriangles");
            ImGui::Begin("BaseTriangles");
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            Render::Navmesh::ImDrawRawTriangles(draw_list, triangles);
            ImGui::End();
        });


    ecs.system<Pathfinding::NavmeshDebugStuff>("RenderNavmesh")
        .kind(flecs::OnUpdate)
        .tick_source(Ticks::tick_render)
        .each([](Pathfinding::NavmeshDebugStuff& navmesh){
            ZoneScopedN("RenderNavmesh");
            ImGui::Begin("Navmesh");
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            Render::Navmesh::ImDrawListPolyMeshDetail(draw_list, *navmesh.polyMeshDetail);
            Render::Navmesh::ImDrawListHeightfieldSolid(draw_list, *navmesh.m_solid);
            ImGui::End();
        });


    // Add GUI components to granary
    // TODO: By using a PreFab, should be able to do this to all buildings of type


    ecs.observer<>("Granary_BuildingCreateAddRenderComponents")
        .event(flecs::OnAdd)
        .with(flecs::IsA).second<Building::Granary_Prefab>()
        //.with<Building::Granary_Prefab>(flecs::IsA)
        .each([](flecs::entity e){
            ZoneScopedN("Granary_BuildingCreateAddRenderComponents");
            e.set<flecs::components::transform::Position3>({0, 0.1, 0});
            e.set<Box>({20, 20, 1});
            e.set<ImColor>(ImColor(ImVec4(255.0 / 255.0, 40.0 / 255.0, 255.0 / 255.0, 1.0f)));
            std::cout << "Added Render Components to Building " << e.path() << std::endl;
            logger->debug("Added Render Components to Building {}", std::string(e.path()));
        });
    

    ecs.observer<>("Tree_CreateAddRenderComponents")
        .event(flecs::OnAdd)
        .with(flecs::IsA).second<Map::Tree_Prefab>()
        .each([](flecs::entity e){
            ZoneScopedN("Tree_CreateAddRenderComponents");
            e.set<flecs::components::transform::Position3>({0, 0, 0});
            e.set<Box>({7, 7, 0});
            e.set<ImColor>(ImColor(ImVec4(0.0 / 255.0, 255.0 / 255.0, 0.0 / 255.0, 1.0f)));
            logger->debug("Added Render Components to Tree {}", std::string(e.path()));
        });

    
    ecs.observer<const Map::GridCellStatic>("GridCell_CreateAddRenderComponents")
        .event(flecs::OnSet)
        .with(flecs::IsA).second<Map::GridCell_Prefab>()
        .each([](flecs::entity e, const Map::GridCellStatic pos){
            ZoneScopedN("GridCell_CreateAddRenderComponents");
            // cells are centered at their coord
            float scale = 20;
            float x = scale*(static_cast<float>(pos.x) - 0.5);
            float y = scale*(static_cast<float>(pos.y) - 0.5);
            e.set<flecs::components::transform::Position3>({x, y, 0});
            e.set<Box>({scale, scale, 0});
            e.set<ImColor>(ImColor(ImVec4(255/ 255.0, 0.0 / 255.0, 0.0 / 255.0, 1.0f)));
            logger->debug("Added Render Components to GridCell {}", std::string(e.path()));
        });
        
    


    // Update render position based on the location
    // TODO: Not sure why this can't be an observer for OnSet
    //ecs.observer<
    ecs.system<
        const Building::Location,
        flecs::components::transform::Position3>("UpdateRenderPosition")
        .term_at(0).in()
        .term_at(1).out()
        //.event(flecs::OnSet)
        .each([](flecs::entity, const Building::Location& loc,flecs::components::transform::Position3& pos){
            ZoneScopedN("UpdateRenderPosition");
            float scale = 20;
            pos.x = scale * (static_cast<float>(loc.x)-0.5);
            pos.y = scale * (static_cast<float>(loc.y)-0.5);
            pos.z = 0;
        });




   
    ecs.observer("Observer_PawnCreate")
        .with(flecs::IsA).second<Pawn::Pawn_Prefab>()
        .event(flecs::OnAdd)
        .each([](flecs::entity pawn){
            ZoneScopedN("Observer_PawnCreate");
            // Set renderable components
            pawn.set<flecs::components::transform::Position3>({0.0, 0.0, 0.0});
            pawn.set<Box>({5, 5, 0});
            pawn.set<ImColor>(ImColor(ImVec4(255/ 255.0, 255.0 / 255.0, 0.0 / 255.0, 1.0f)));
        });
    



    
    auto updatePawnRenderLocation_sys = ecs.system<
        Coordinates::Grid,
        Coordinates::Cell,
        flecs::components::transform::Position3>("Update Pawn Render Location")
    .term_at(0).in()
    .term_at(1).in()
    .term_at(2).out()
    .tick_source(Ticks::tick_render)
    .each([](flecs::entity pawn, const Coordinates::Grid& grid, const Coordinates::Cell& cell, flecs::components::transform::Position3& renderPos){
        float scale = 20;
        renderPos.x = scale*(grid.x -0.5 + cell.x/2);  // -0.5 because centre of the cell
        renderPos.y = scale*(grid.y -0.5 + cell.y/2);
    });
    











    







};


}