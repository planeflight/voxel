#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"
#include "omega/core/app.hpp"
#include "imgui/imgui.h"
#include "omega/core/viewport.hpp"
#include "omega/events/event.hpp"
#include "omega/gfx/frame_buffer.hpp"
#include "omega/gfx/gl.hpp"
#include "omega/gfx/renderer/dfr.hpp"
#include "omega/gfx/texture/texture.hpp"
#include "omega/scene/imgui.hpp"
#include "omega/scene/perspective_camera.hpp"
#include "omega/util/std.hpp"
#include "omega/util/time.hpp"
#include "voxel/entity/chunk.hpp"
#include "voxel/entity/player.hpp"

using namespace omega;

struct VoxelGame : public core::App {
    VoxelGame(const core::AppConfig &config) : core::App::App(config) {

    }
    
    void setup() override {
        // gfx::enable_blending();
        gfx::set_depth_test(true);

        globals->input.set_relative_mouse_mode(true);
        player = util::create_uptr<Player>(math::vec3(5.0f), math::vec3(1.0f));
        viewport = util::create_uptr<core::Viewport>(
            core::ViewportType::fit, 1600, 900);
        viewport->on_resize(window->get_width(), window->get_height());

        chunks.push_back(
            util::create_uptr<Chunk>(math::vec3(0.0f)));

        // load shaders
        globals->asset_manager.load_shader("block", "./res/shaders/block.glsl");
        globals->asset_manager.load_shader("composite", "./res/shaders/composite.glsl");
        globals->asset_manager.load_shader("shadow_map", "./res/shaders/shadow_map.glsl");

        // load textures
        globals->asset_manager.load_texture("block", "./res/textures/blocks.png");

        // load framebuffers and deferred renderer
        std::vector<gfx::FrameBufferAttachment> attachments{
            {
                .width = 1600,
                .height = 900,
                .name = "position",
                .internal_fmt = gfx::texture::TextureFormat::RGBA_32F,
                .external_fmt = gfx::texture::TextureFormat::RGBA,
                .min_filter = gfx::texture::TextureParam::LINEAR,
                .mag_filter = gfx::texture::TextureParam::LINEAR,
            },
            {
                .width = 1600,
                .height = 900,
                .name = "normal",
                .internal_fmt = gfx::texture::TextureFormat::RGBA_32F,
                .external_fmt = gfx::texture::TextureFormat::RGBA,
                .min_filter = gfx::texture::TextureParam::LINEAR,
                .mag_filter = gfx::texture::TextureParam::LINEAR,
            },
            {
                .width = 1600,
                .height = 900,
                .name = "color_spec",
                .internal_fmt = gfx::texture::TextureFormat::RGBA,
                .external_fmt = gfx::texture::TextureFormat::RGBA,
                .min_filter = gfx::texture::TextureParam::LINEAR,
                .mag_filter = gfx::texture::TextureParam::LINEAR,
            }
        };

        dfr = util::create_uptr<gfx::renderer::DeferredRenderer>(
            1600, 900, attachments);

        attachments.clear();
        attachments.push_back({
            .width = 1600,
            .height = 900,
            .name = "shadow_map",
            .internal_fmt = gfx::texture::TextureFormat::DEPTH_COMPONENT,
            .external_fmt = gfx::texture::TextureFormat::DEPTH_COMPONENT,
            .min_filter = gfx::texture::TextureParam::LINEAR,
            .mag_filter = gfx::texture::TextureParam::LINEAR,
            .wrap_s = gfx::texture::TextureParam::CLAMP_TO_BORDER,
            .wrap_t = gfx::texture::TextureParam::CLAMP_TO_BORDER,
            .draw_buffer = false
        });
        shadow_map = util::create_uptr<gfx::FrameBuffer>(1600, 900, attachments);
    }
    
    void render(f32 dt) override {
        // render to g buffer first
        dfr->geometry_pass([&](gfx::FrameBuffer *fbo) {
            gfx::set_clear_color(135.0f / 255.0f, 206 / 255.0f, 235 / 255.0f, 1.0f);
            gfx::clear_buffer(
                OMEGA_GL_COLOR_BUFFER_BIT | OMEGA_GL_DEPTH_BUFFER_BIT);

            player->recalculate_view_matrix();
            auto *shader = globals->asset_manager.get_shader("block");
            shader->bind();
            shader->set_uniform_mat4f("u_view", player->get_view_matrix());
            shader->set_uniform_mat4f("u_projection", player->get_projection_matrix());

            globals->asset_manager.get_texture("block")->bind(0);
            shader->set_uniform_1i("u_texture", 0);

            shader->set_uniform_3f("u_chunk_size", Chunk::dimens.x, Chunk::dimens.y, Chunk::dimens.z);
            for (auto &chunk : chunks) {
                shader->set_uniform_3f("u_chunk_offset", chunk->get_position().x, chunk->get_position().y, chunk->get_position().z);
                chunk->render(dt);
            }
            shader->unbind();
        });


        // render to shadow map
        shadow_map->bind();
        gfx::viewport(0, 0, shadow_map->get_width(), shadow_map->get_height());
        gfx::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        gfx::clear_buffer(OMEGA_GL_DEPTH_BUFFER_BIT);
        auto *shadow_map_shader = globals->asset_manager.get_shader("shadow_map");
        shadow_map_shader->bind();
        float near = -100.0f, far = 100.0f;
        glm::vec3 light_pos{10.0f};
        math::mat4 light_projection = math::ortho(near, far, near, far, near, far);
        math::mat4 light_view = math::lookAt(light_pos, math::vec3(0.0f), math::vec3(0.0f, 1.0f, 0.0f));
        math::mat4 light_space = light_projection * light_view;

        shadow_map_shader->set_uniform_3f("u_chunk_size", Chunk::dimens.x, Chunk::dimens.y, Chunk::dimens.z);
        shadow_map_shader->set_uniform_mat4f("u_light_space", light_space);

        for (auto &chunk : chunks) {
            shadow_map_shader->set_uniform_3f("u_chunk_offset", chunk->get_position().x, chunk->get_position().y, chunk->get_position().z);
            chunk->render(dt);
        }
        shadow_map_shader->unbind();

        gfx::FrameBuffer::unbind();

        // render to composite
        dfr->composite_pass([&](gfx::FrameBuffer *fbo) {
            gfx::set_clear_color(1.0f, 1.0f, 1.0f, 1.0f);
            gfx::clear_buffer(
                OMEGA_GL_COLOR_BUFFER_BIT | OMEGA_GL_DEPTH_BUFFER_BIT);
            viewport->on_resize(window->get_width(), window->get_height());
            auto *composite_shader = globals->asset_manager.get_shader("composite");
            composite_shader->bind();

            fbo->get_attachment("position").bind(0);
            fbo->get_attachment("normal").bind(1);
            fbo->get_attachment("color_spec").bind(2);

            shadow_map->get_attachment("shadow_map").bind(3);

            composite_shader->set_uniform_1i("u_position", 0);
            composite_shader->set_uniform_1i("u_normal", 1);
            composite_shader->set_uniform_1i("u_color", 2);
            composite_shader->set_uniform_1i("u_depth_map", 3);
            composite_shader->set_uniform_3f("u_view_pos", player->position.x, player->position.y, player->position.z);
            composite_shader->set_uniform_mat4f("u_light_space", light_space);
            // sun uniforms
            composite_shader->set_uniform_3f("u_sunlight.direction", -light_pos.x, -light_pos.y, -light_pos.z);
            composite_shader->set_uniform_3f("u_sunlight.ambient", 0.6f, 0.6f, 0.6f);
            composite_shader->set_uniform_3f("u_sunlight.diffuse", 1.0f, 1.0f, 1.0f);
            composite_shader->set_uniform_3f("u_sunlight.specular", 1.0f, 1.0f, 1.0f);
        });

    }

    void update(f32 dt) override {
        player->update(dt, -25.0f);
        player->handle_collisions(dt, chunks[0].get());
    }
    
    void input(f32 dt) override {
        auto &keys = globals->input.get_key_manager();
        if (keys.key_pressed(events::Key::k_escape)) {
            running = false;
        }
        // update camera
        // forwards
        if (keys.key_pressed(omega::events::Key::k_w) || keys.key_pressed(omega::events::Key::k_up)) {
            player->position += player->get_front() * player_speed * dt;
        } // backwards
        else if (keys.key_pressed(omega::events::Key::k_s) || keys.key_pressed(omega::events::Key::k_down)) {
            player->position -= player->get_front() * player_speed * dt;
        } // left
        else if (keys.key_pressed(omega::events::Key::k_a) || keys.key_pressed(omega::events::Key::k_left)) {
            player->position -= player->get_right() * player_speed * dt;
        } // right
        else if (keys.key_pressed(omega::events::Key::k_d) || keys.key_pressed(omega::events::Key::k_right)) {
            player->position += player->get_right() * player_speed * dt;
        }

        // jump
        if (keys.key_just_pressed(omega::events::Key::k_space)) {
            player->velocity.y = 10.0f;
        }

        glm::vec2 mouse_move = globals->input.get_mouse_move();
        player->mouse_movement(mouse_move.x, mouse_move.y, 0.1f);
        // enable/disable mouse
        if (keys.key_just_released(omega::events::Key::k_m)) {
            globals->input.set_relative_mouse_mode(!globals->input.get_relative_mouse_mode());
        }

    }

    void on_resize(u32 width, u32 height) override {
        viewport->on_resize(width, height);
    }

    util::uptr<Player> player = nullptr;
    util::uptr<core::Viewport> viewport = nullptr;
    std::vector<util::uptr<Chunk>> chunks;

    constexpr static float player_speed = 10.0f;


    util::uptr<gfx::renderer::DeferredRenderer> dfr = nullptr;
    util::uptr<gfx::FrameBuffer> shadow_map = nullptr;
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

