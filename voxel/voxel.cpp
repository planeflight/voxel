#include "omega/core/app.hpp"
#include "imgui/imgui.h"
#include "omega/core/viewport.hpp"
#include "omega/events/event.hpp"
#include "omega/gfx/gl.hpp"
#include "omega/scene/imgui.hpp"
#include "omega/scene/perspective_camera.hpp"
#include "omega/util/std.hpp"
#include "voxel/entity/chunk.hpp"

using namespace omega;

struct VoxelGame : public core::App {
    VoxelGame(const core::AppConfig &config) : core::App::App(config) {

    }
    
    void setup() override {
        gfx::enable_blending();
        gfx::set_depth_test(true);

        globals->input.set_relative_mouse_mode(true);
        camera = util::create_uptr<scene::PerspectiveCamera>(
            math::vec3(0.0f), -90.0f, 0.0f);
        viewport = util::create_uptr<core::Viewport>(
            core::ViewportType::fit, 1600, 900);
        viewport->on_resize(window->get_width(), window->get_height());

        chunks.push_back(
            util::create_uptr<Chunk>(math::vec3(0.0f)));

        // load shaders
        globals->asset_manager.load_shader("block", "./res/shaders/block.glsl");

        // load textures
        globals->asset_manager.load_texture("block", "./res/textures/blocks.png");
    }
    
    void render(f32 dt) override {
        gfx::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        gfx::clear_buffer(
            OMEGA_GL_COLOR_BUFFER_BIT | OMEGA_GL_DEPTH_BUFFER_BIT);

        camera->recalculate_view_matrix();
        auto *shader = globals->asset_manager.get_shader("block");
        shader->bind();
        shader->set_uniform_mat4f("u_view", camera->get_view_matrix());
        shader->set_uniform_mat4f("u_projection", camera->get_projection_matrix());
        
        globals->asset_manager.get_texture("block")->bind(0);
        shader->set_uniform_1i("u_texture", 0);

        for (auto &chunk : chunks) {
            chunk->render(dt);
        }
        shader->unbind();
    }

    void update(f32 dt) override {

    }
    
    void input(f32 dt) override {
        auto &keys = globals->input.get_key_manager();
        if (keys.key_pressed(events::Key::k_escape)) {
            running = false;
        }
        // update camera
        // forwards
        if (keys.key_pressed(omega::events::Key::k_w) || keys.key_pressed(omega::events::Key::k_up)) {
            camera->position += camera->get_front() * camera_speed * dt;
        } // backwards
        else if (keys.key_pressed(omega::events::Key::k_s) || keys.key_pressed(omega::events::Key::k_down)) {
            camera->position -= camera->get_front() * camera_speed * dt;
        } // left
        else if (keys.key_pressed(omega::events::Key::k_a) || keys.key_pressed(omega::events::Key::k_left)) {
            camera->position -= camera->get_right() * camera_speed * dt;
        } // right
        else if (keys.key_pressed(omega::events::Key::k_d) || keys.key_pressed(omega::events::Key::k_right)) {
            camera->position += camera->get_right() * camera_speed * dt;
        }

        glm::vec2 mouse_move = globals->input.get_mouse_move();
        camera->mouse_movement(mouse_move.x, mouse_move.y, 0.1f);
        // enable/disable mouse
        if (keys.key_just_released(omega::events::Key::k_m)) {
            globals->input.set_relative_mouse_mode(!globals->input.get_relative_mouse_mode());
        }

    }

    void on_resize(u32 width, u32 height) override {
        viewport->on_resize(width, height);
    }

    util::uptr<scene::PerspectiveCamera> camera = nullptr;
    util::uptr<core::Viewport> viewport = nullptr;
    std::vector<util::uptr<Chunk>> chunks;

    constexpr static float camera_speed = 10.0f;
};

int main() {
    core::AppConfig config;
    config.resizable = true;
    config.width = 1920;
    config.height = 1080;
    config.imgui = true;
    config.title = "GAME";

    VoxelGame app{config};
    app.run();
}

