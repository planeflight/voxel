#include "imgui/imgui.h"
#include "omega/core/app.hpp"
#include "omega/core/viewport.hpp"
#include "omega/events/event.hpp"
#include "omega/gfx/frame_buffer.hpp"
#include "omega/gfx/gl.hpp"
#include "omega/gfx/renderer/dfr.hpp"
#include "omega/gfx/texture/texture.hpp"
#include "omega/math/bezier.hpp"
#include "omega/scene/imgui.hpp"
#include "omega/scene/perspective_camera.hpp"
#include "omega/util/random.hpp"
#include "omega/util/std.hpp"
#include "omega/util/time.hpp"
#include "voxel/entity/chunk.hpp"
#include "voxel/entity/player.hpp"
#include "voxel/entity/sun.hpp"

using namespace omega;

struct VoxelGame : public core::App {
    VoxelGame(const core::AppConfig &config) : core::App::App(config) {}

    void setup() override {
        util::seed_time();
        // gfx::enable_blending();
        gfx::set_depth_test(true);

        globals->input.set_relative_mouse_mode(true);
        player = util::create_uptr<Player>(math::vec3(0.0f, 0.0f, 0.0f),
                                           math::vec3(1.0f));
        player->set_projection(45.0f, 1600.0f / 900.0f, 1.0f, 128.0f);
        // create sun
        sun = util::create_uptr<Sun>();
        // sun->direction = math::normalize(math::vec3(0.60f, -0.7f, -0.30f));
        sun->direction = math::normalize(math::vec3(-7.0f, -10.0f, 4.6f));
        sun->ambient = math::vec3(0.45f);
        sun->diffuse = math::vec3(1.0f);

        viewport = util::create_uptr<core::Viewport>(
            core::ViewportType::fit, 1600, 900);
        viewport->on_resize(window->get_width(), window->get_height());

        // push chunks
        for (i32 z = 0; z < 4; ++z) {
            for (i32 x = 0; x < 4; ++x) {
                chunks.push_back(
                    util::create_uptr<Chunk>(math::vec3((f32)x, 0.0f, (f32)z)));
            }
        }

        // load shaders
        globals->asset_manager.load_shader("block", "./res/shaders/block.glsl");
        globals->asset_manager.load_shader("composite",
                                           "./res/shaders/composite.glsl");
        globals->asset_manager.load_shader("shadow_map",
                                           "./res/shaders/shadow_map.glsl");
        globals->asset_manager.load_shader("ssao", "./res/shaders/ssao.glsl");
        globals->asset_manager.load_shader("ssao_blur",
                                           "./res/shaders/ssao_blur.glsl");

        // load textures
        globals->asset_manager.load_texture("block",
                                            "./res/textures/blocks.png");

        // load SSAO noise texture
        auto *noise_texture = globals->asset_manager.load_empty_texture(
            "ssao_noise",
            4,
            4,
            gfx::texture::TextureParam::NEAREST,
            gfx::texture::TextureParam::NEAREST);
        std::array<u8, 4 * 4 * 4> noise_data;
        for (u32 i = 0; i < noise_data.size(); i += 4) {
            noise_data[i + 0] = util::random<u8>(0, 255);
            noise_data[i + 1] = util::random<u8>(0, 255);
            noise_data[i + 2] = 0;
            noise_data[i + 3] = 255;
        }
        noise_texture->set_data((u32 *)noise_data.data());
        glBindTexture(GL_TEXTURE_2D, noise_texture->get_renderer_id());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        // load SSAO noise random rotation vectors
        std::array<math::vec3, 64> rotation_vectors;
        for (u32 i = 0; i < 64; ++i) {
            math::vec3 v = {util::random<f32>(-1.0f, 1.0f),
                            util::random<f32>(-1.0f, 1.0f),
                            util::random<f32>(0.0f, 1.0f)};
            // make a semi-sphere
            v = math::normalize(v);
            v *= util::random<f32>(0.0f, 1.0f);
            // make values closer to the center on average w/ quadratic function
            f32 scale = (f32)i / 64.0f;
            scale = math::lerp(0.1f, 1.0f, scale * scale);
            v *= scale;

            rotation_vectors[i] = v;
        }
        auto *ssao_shader = globals->asset_manager.get_shader("ssao");
        ssao_shader->bind();
        for (u32 i = 0; i < 64; ++i) {
            ssao_shader->set_uniform_3f(
                "u_ssao_samples[" + std::to_string(i) + "]",
                rotation_vectors[i]);
        }
        ssao_shader->unbind();

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
            }};

        dfr = util::create_uptr<gfx::renderer::DeferredRenderer>(
            1600, 900, attachments);
        // add SSAO framebuffer
        attachments.clear();
        attachments.push_back({
            .width = 1600,
            .height = 900,
            .name = "ssao",
            .internal_fmt = gfx::texture::TextureFormat::RGBA,
            .external_fmt = gfx::texture::TextureFormat::RGBA,
            .min_filter = gfx::texture::TextureParam::LINEAR,
            .mag_filter = gfx::texture::TextureParam::LINEAR,
        });
        dfr->framebuffers["ssao"] =
            util::create_uptr<gfx::FrameBuffer>(1600, 900, attachments);

        // add SSAO_blur framebuffer
        attachments.clear();
        attachments.push_back({
            .width = 1600,
            .height = 900,
            .name = "ssao_blur",
            .internal_fmt = gfx::texture::TextureFormat::RED,
            .external_fmt = gfx::texture::TextureFormat::RED,
            .min_filter = gfx::texture::TextureParam::LINEAR,
            .mag_filter = gfx::texture::TextureParam::LINEAR,
        });
        dfr->framebuffers["ssao_blur"] =
            util::create_uptr<gfx::FrameBuffer>(1600, 900, attachments);

        // TODO: make it use less memory
        const u32 shadow_map_size = 2048;
        attachments.clear();
        attachments.push_back(
            {.width = shadow_map_size,
             .height = shadow_map_size,
             .name = "shadow_map",
             .internal_fmt = gfx::texture::TextureFormat::DEPTH_COMPONENT,
             .external_fmt = gfx::texture::TextureFormat::DEPTH_COMPONENT,
             .min_filter = gfx::texture::TextureParam::LINEAR,
             .mag_filter = gfx::texture::TextureParam::LINEAR,
             .wrap_s = gfx::texture::TextureParam::CLAMP_TO_BORDER,
             .wrap_t = gfx::texture::TextureParam::CLAMP_TO_BORDER,
             .draw_buffer = false});
        shadow_map = util::create_uptr<gfx::FrameBuffer>(
            shadow_map_size, shadow_map_size, attachments);
    }

    void render(f32 dt) override {
        // render to g buffer first
        dfr->geometry_pass([&]() {
            // make black to prevent leaking into gbuffer
            gfx::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
            gfx::clear_buffer(OMEGA_GL_COLOR_BUFFER_BIT |
                              OMEGA_GL_DEPTH_BUFFER_BIT);

            player->recalculate_view_matrix();
            auto *shader = globals->asset_manager.get_shader("block");
            shader->bind();
            shader->set_uniform_mat4f("u_view", player->get_view_matrix());
            shader->set_uniform_mat4f("u_projection",
                                      player->get_projection_matrix());

            globals->asset_manager.get_texture("block")->bind(0);
            shader->set_uniform_1i("u_texture", 0);

            shader->set_uniform_3f("u_chunk_size", Chunk::dimens);
            for (auto &chunk : chunks) {
                shader->set_uniform_3f("u_chunk_offset", chunk->get_position());
                chunk->render(dt);
            }
            shader->unbind();
        });

        // render to shadow map
        shadow_map->bind();
        gfx::viewport(0, 0, shadow_map->get_width(), shadow_map->get_height());
        gfx::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
        gfx::clear_buffer(OMEGA_GL_DEPTH_BUFFER_BIT);
        auto *shadow_map_shader =
            globals->asset_manager.get_shader("shadow_map");
        shadow_map_shader->bind();
        shadow_map_shader->set_uniform_3f("u_chunk_size", Chunk::dimens);
        shadow_map_shader->set_uniform_mat4f("u_light_space",
                                             sun->get_view_projection_matrix());

        for (auto &chunk : chunks) {
            shadow_map_shader->set_uniform_3f("u_chunk_offset",
                                              chunk->get_position());
            chunk->render(dt);
        }
        shadow_map_shader->unbind();

        gfx::FrameBuffer::unbind();

        // render SSAO
        dfr->quad_pass([&]() {
            auto ssao_fbo = dfr->framebuffers["ssao"].get();
            ssao_fbo->bind();
            gfx::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
            gfx::clear_buffer(OMEGA_GL_COLOR_BUFFER_BIT |
                              OMEGA_GL_DEPTH_BUFFER_BIT);

            auto ssao_shader = globals->asset_manager.get_shader("ssao");
            ssao_shader->bind();
            // bind gbuffer textures
            dfr->gbuffer->get_attachment("position").bind(0);
            dfr->gbuffer->get_attachment("normal").bind(1);
            // bind noise texture
            globals->asset_manager.get_texture("ssao_noise")->bind(2);

            // set uniforms
            ssao_shader->set_uniform_1i("u_position", 0);
            ssao_shader->set_uniform_1i("u_normal", 1);
            ssao_shader->set_uniform_1i("u_noise", 2);

            ssao_shader->set_uniform_mat4f("u_projection",
                                           player->get_projection_matrix());
            ssao_shader->set_uniform_mat4f("u_view", player->get_view_matrix());
        });

        // blur SSAO
        dfr->quad_pass([&]() {
            auto ssao_blur_fbo = dfr->framebuffers["ssao_blur"].get();
            ssao_blur_fbo->bind();
            gfx::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
            gfx::clear_buffer(OMEGA_GL_COLOR_BUFFER_BIT |
                              OMEGA_GL_DEPTH_BUFFER_BIT);
            auto *shader = globals->asset_manager.get_shader("ssao_blur");
            // bind ssao texture
            dfr->framebuffers["ssao"]->get_attachment("ssao").bind(0);
            shader->set_uniform_1i("u_ssao", 0);
        });

        gfx::FrameBuffer::unbind();

        // render to composite
        dfr->quad_pass([&]() {
            gfx::FrameBuffer *fbo = dfr->gbuffer.get();
            gfx::set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
            gfx::clear_buffer(OMEGA_GL_COLOR_BUFFER_BIT |
                              OMEGA_GL_DEPTH_BUFFER_BIT);
            viewport->on_resize(window->get_width(), window->get_height());
            auto *composite_shader =
                globals->asset_manager.get_shader("composite");
            composite_shader->bind();

            // bind gbuffer textures
            fbo->get_attachment("position").bind(0);
            fbo->get_attachment("normal").bind(1);
            fbo->get_attachment("color_spec").bind(2);

            // bind shadow map texture
            shadow_map->get_attachment("shadow_map").bind(3);

            // bind ssao blur texture
            dfr->framebuffers["ssao_blur"]->get_attachment("ssao_blur").bind(4);

            composite_shader->set_uniform_1i("u_position", 0);
            composite_shader->set_uniform_1i("u_normal", 1);
            composite_shader->set_uniform_1i("u_color", 2);
            composite_shader->set_uniform_1i("u_depth_map", 3);
            composite_shader->set_uniform_1i("u_ssao", 4);

            composite_shader->set_uniform_3f("u_view_pos", player->position);
            composite_shader->set_uniform_mat4f(
                "u_light_space", sun->get_view_projection_matrix());
            // sun uniforms
            composite_shader->set_uniform_3f("u_sunlight.direction",
                                             sun->direction);
            composite_shader->set_uniform_3f("u_sunlight.ambient",
                                             sun->ambient);
            composite_shader->set_uniform_3f("u_sunlight.diffuse",
                                             sun->diffuse);
            composite_shader->set_uniform_3f("u_sunlight.specular",
                                             sun->diffuse);
        });
    }

    void update(f32 dt) override {
        player->update(dt, -25.0f);
        player->handle_collisions(dt, chunks[0].get());

        sun->update_camera(*player);
    }

    void input(f32 dt) override {
        auto &keys = globals->input.get_key_manager();
        if (keys.key_pressed(events::Key::k_escape)) {
            running = false;
        }
        // update camera
        // forwards
        if (keys.key_pressed(omega::events::Key::k_w) ||
            keys.key_pressed(omega::events::Key::k_up)) {
            player->position += player->get_front() * player_speed * dt;
        } // backwards
        else if (keys.key_pressed(omega::events::Key::k_s) ||
                 keys.key_pressed(omega::events::Key::k_down)) {
            player->position -= player->get_front() * player_speed * dt;
        } // left
        else if (keys.key_pressed(omega::events::Key::k_a) ||
                 keys.key_pressed(omega::events::Key::k_left)) {
            player->position -= player->get_right() * player_speed * dt;
        } // right
        else if (keys.key_pressed(omega::events::Key::k_d) ||
                 keys.key_pressed(omega::events::Key::k_right)) {
            player->position += player->get_right() * player_speed * dt;
        }

        // jump
        if (keys.key_just_pressed(omega::events::Key::k_space)) {
            player->velocity.y = 10.0f;
        }

        glm::vec2 mouse_move = globals->input.get_mouse_move();
        mouse_move *= globals->input.get_mouse_sensitivity();
        player->mouse_movement(mouse_move.x, mouse_move.y);
        // enable/disable mouse
        if (keys.key_just_released(omega::events::Key::k_m)) {
            globals->input.set_relative_mouse_mode(
                !globals->input.get_relative_mouse_mode());
        }
    }

    void on_resize(u32 width, u32 height) override {
        viewport->on_resize(width, height);
    }

    util::uptr<Player> player = nullptr;
    util::uptr<Sun> sun = nullptr;
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
