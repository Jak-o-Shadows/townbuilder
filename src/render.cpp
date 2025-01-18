#include "render.hpp"

#include <tracy/Tracy.hpp>
#include <spdlog/spdlog.h>

#include "flecs_components_transform.h"
#include "flecs_components_graphics.h"
#include "flecs_components_geometry.h"
#include "flecs_components_physics.h"
#include "flecs_components_gui.h"
#include "flecs_components_input.h"
#include "flecs_systems_transform.h"
#include "flecs_systems_physics.h"

#include "imgui.h"
#include "../bindings/imgui_impl_glfw.h"
#include "../bindings/imgui_impl_opengl3.h"



#include "componentsPawn.hpp"
#include "componentsBuilding.hpp"
#include "ticks.hpp"

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



struct Game {
    flecs::entity window;
    flecs::entity level;
    
    flecs::components::transform::Position3 center;
    float size;        
};




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


    /*
    // Initialise game
    const float TileSize = 3.0;
    const float TileHeight = 0.5;
    const float PathHeight = 0.1;
    const float TileSpacing = 0.00;
    Game& g = ecs.ensure<Game>();
    g.center = {0, 0, 0};//{ to_x(map_width / 2), 0, to_z(map_height / 2) };
    // Get the map entity back out for working with for the moment
    /*
    const Map::Grid* map = Map::mapEntity.get<Map::Grid>();
    std::cout << "Map Object gotten" << std::endl;
    int map_width = map->m_width;
    int map_height = map->m_height;
    g.size = map_width * (TileSize + TileSpacing) + 2;
    std::cout << "map GUI setup" << std::endl;
    */
   /*
   logger->trace("Map Gui Setup");
   std::cout << "Map Gui Setup" << std::endl;

    
    // Cannot figure out how to move these to render - so stuff it
    // Init UI
    
    flecs::components::graphics::Camera camera_data = {};
    camera_data.set_up(0, 1, 0);
    camera_data.set_fov(20);
    camera_data.near_ = 1.0;
    camera_data.far_ = 100.0;
    //auto camera = ecs.entity("Camera")
    auto camera = flecs::entity(ecs, "Camera");
        camera.set<flecs::components::transform::Position3>({0, 8.0, -9.0});
        camera.set<flecs::components::transform::Rotation3>({-0.5});
        camera.set<flecs::components::graphics::Camera>(camera_data);
        camera.add(flecs::game::CameraController);
    logger->trace("Camera Created");
    std::cout << "Camera Created" << std::endl;
    

    flecs::components::graphics::DirectionalLight light_data = {};
    light_data.set_direction(0.3, -1.0, 0.5);
    light_data.set_color(0.98, 0.95, 0.8);
    light_data.intensity = 0.01;
    //auto light = ecs.entity("Sun")
    auto light = flecs::entity(ecs, "Sun")
        .set<flecs::components::graphics::DirectionalLight>(light_data);
    logger->trace("Light Created");
    std::cout << "Light Created" << std::endl;

    
    flecs::components::gui::Canvas canvas_data = {};
    canvas_data.width = 800;
    canvas_data.height = 600;
    canvas_data.title = (char*)"TownBuilder";
    canvas_data.camera = camera.id();
    canvas_data.directional_light = light.id();
    canvas_data.ambient_light = {0.006, 0.005, 0.018};
    canvas_data.background_color = {0.15, 0.4, 0.6};
    canvas_data.fog_density = 1.0;
    //ecs.entity()
    flecs::entity(ecs, "Canvas_asdf")
        .set<flecs::components::gui::Canvas>(canvas_data);
    logger->trace("Canvas Created");
    std::cout << "Canvas Created" << std::endl;
    












    // Add GUI components to granary
    // TODO: By using a PreFab, should be able to do this to all buildings of type
    /*
    ecs.observer<flecs::ChildOf>("Observer_BuildingCreate")
        .term_at(0).second(Building::buildingsParent)
        .event(flecs::OnAdd)
        .each([](flecs::entity building){
            ZoneScopedN("Observer_BuildingCreate");
            const Building::Location* loc = building.get<Building::Location>();
            const Building::BuildingUI* size = building.get<Building::BuildingUI>();
            building.set<flecs::components::transform::Position3>({(float) loc->x, 0.1, (float) loc->y});
            building.set<flecs::components::geometry::Box>({(float) size->sizeX, 2, (float) size->sizeY});
            building.set<flecs::components::graphics::Color>({20, 0, 0});
        });
    */

   /*
    ecs.observer("Observer_PawnCreate")
        .with(flecs::ChildOf, Pawn::pawnsParent)
        .event(flecs::OnAdd)
        .each([](flecs::entity pawn){
            ZoneScopedN("Observer_PawnCreate");
            std::cout << "Pawn Creation Observer " << pawn.name() << std::endl;
            // Get the location
            flecs::entity currentCell = pawn.target<Pawn::PawnOccupying>();
            const GridCellStatic* loc = currentCell.get<GridCellStatic>();
            // Then set renderable components
            pawn.set<flecs::components::transform::Position3>({(float) loc->x, 0.1, (float) loc->y});
            pawn.set<flecs::components::geometry::Box>({0.1, 0.8, 0.1});
            pawn.set<flecs::components::graphics::Color>({165, 42, 42});          
        });
    */

    /*
    // Add rendering components to Pawns
    //  As iterating, must defer
    // TODO: This should be an observer looking for when new pawns are added to the pawnsParent
    ecs.defer_begin();
    Pawn::pawnsParent.children([](flecs::entity pawn) {

        });
    ecs.defer_end();
    */



    /*
    auto updatePawnRenderLocation_sys = ecs.system<Pawn::Position, flecs::components::transform::Position3>("Update Pawn Render Location")
    .tick_source(Ticks::tick_render)
    .with<Pawn::PawnOccupying>(flecs::Wildcard)
    .each([](flecs::entity pawn, Pawn::Position& p, flecs::components::transform::Position3& renderPos){
        // Get cell from pawn occupying
        flecs::entity currentCell = pawn.target<Pawn::PawnOccupying>();
        const GridCellStatic* loc = currentCell.get<GridCellStatic>();
        renderPos.x = loc->x + p.x/2;
        renderPos.z = loc->y + p.y/2;
    });
    */











    







};


}