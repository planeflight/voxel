#ifndef VOXEL_ENTITY_SUN_HPP
#define VOXEL_ENTITY_SUN_HPP

#include <array>
#include <cmath>
#include <utility>

#include "omega/scene/orthographic_camera.hpp"
#include "omega/util/time.hpp"
#include "voxel/entity/player.hpp"

struct Sun : omega::scene::OrthographicCamera {
    omega::math::vec3 direction{0.0f};

    struct SunState {
        enum { DAY = 0, DUSK = 1, NIGHT = 2, DAWN = 3 } state = DAY;
        omega::math::vec3 ambient{0.0f};
        omega::math::vec3 diffuse{0.0f};
        SunState *next = nullptr, *prev = nullptr;
    };

    std::vector<SunState> states;
    SunState current_state;

    // shadow box constants
    constexpr static float near = -20.0f, far = 20.0f;
    constexpr static float shadow_distance = 512.0f;
    // shadow box minimum and maxiumum
    omega::math::vec3 min{0.0f}, max{0.0f};

    Sun() : OrthographicCamera(near, far, near, far, near, far) {
        // setup states that link to each other
        states.push_back(Sun::SunState{.state = Sun::SunState::DAY,
                                       .ambient = omega::math::vec3(0.45f),
                                       .diffuse = omega::math::vec3(1.0f)});
        states.push_back(Sun::SunState{.state = Sun::SunState::DUSK,
                                       .ambient = omega::math::vec3(0.25f),
                                       .diffuse = omega::math::vec3(0.7f)});
        states.push_back(Sun::SunState{.state = Sun::SunState::NIGHT,
                                       .ambient = omega::math::vec3(0.1f),
                                       .diffuse = omega::math::vec3(0.0f)});
        states.push_back(Sun::SunState{.state = Sun::SunState::DAWN,
                                       .ambient = omega::math::vec3(0.25f),
                                       .diffuse = omega::math::vec3(0.6f)});
        states[0].next = &states[1];
        states[1].next = &states[2];
        states[2].next = &states[3];
        states[3].next = &states[0];

        states[0].prev = &states[3];
        states[1].prev = &states[0];
        states[2].prev = &states[1];
        states[3].prev = &states[2];

        current_state = states[0];
        direction =
            omega::math::normalize(omega::math::vec3(-7.0f, -10.0f, 4.6f));
    }

    void update_camera(Player &player) {
        using namespace omega;

        const f32 frustum = 512.0f;
        const math::vec2 min{-frustum / 2.0f}, max{frustum / 2.0f};
        projection_matrix = math::ortho(min.x,
                                        max.x,
                                        min.y,
                                        max.y,
                                        -shadow_distance / 2.0f,
                                        shadow_distance / 2.0f);

        player.recalculate_view_matrix();
        auto inv_vp = math::inverse(player.get_view_projection_matrix());

        // NDC to world space
        auto w_c = inv_vp * math::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        w_c *= 2.0f;
        // w_c /= w_c.w;
        // to avoid the jittering, round to the nearest 2 texels
        // w_c = math::round(w_c * 0.5f) * 2.0f;
        // w_c /= w_c.w;
        // util::debug("{}", math::to_string(w_c));
        math::vec3 center{0.0f}, eye{0.0f}, up{0.0f, 1.0f, 0.0f};
        center = w_c;
        eye = center - direction;
        view_matrix = math::lookAt(eye, center, up);
    }

    void update_lighting() {
        using namespace omega::math;
        f32 t = omega::util::time::get_time<f32>() * 0.05f;
        f32 cos_t = cos(t);
        direction = vec3(cos_t, sin(t), 1.0f) * vec3(-7.0f, -10.0f, 4.6f);

        // get [0-1], scale quadratically, and set the state
        f32 loop_t = cos_t * 0.5f + 0.5f;
        loop_t *= loop_t;
        current_state.ambient =
            lerp(states[0].ambient, states[2].ambient, loop_t);
        current_state.diffuse =
            lerp(states[0].diffuse, states[2].diffuse, loop_t);
    }
};

#endif // VOXEL_ENTITY_SUN_HPP
