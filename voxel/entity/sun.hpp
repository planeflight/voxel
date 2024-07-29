#ifndef VOXEL_ENTITY_SUN_HPP
#define VOXEL_ENTITY_SUN_HPP

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "omega/math/math.hpp"
#include "omega/scene/orthographic_camera.hpp"
#include "voxel/entity/player.hpp"

struct Sun : omega::scene::OrthographicCamera {
    omega::math::vec3 direction{0.0f};
    omega::math::vec3 ambient{0.0f};
    omega::math::vec3 diffuse{0.0f};

    constexpr static float near = -20.0f, far = 20.0f;
    constexpr static float shadow_distance = 256.0f;

    Sun()
        : omega::scene::OrthographicCamera::OrthographicCamera(near,
                                                               far,
                                                               near,
                                                               far,
                                                               near,
                                                               far) {}

    omega::math::vec3 min{0.0f}, max{0.0f};

    void update_camera(Player &player) {
        using namespace omega;

        const f32 frustum = 256.0f;
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

    std::vector<omega::math::vec4> get_frustum_corners(
        const omega::math::mat4 &view_proj) {
        using namespace omega;
        math::mat4 inv = math::inverse(view_proj);
        std::vector<math::vec4> corners;
        for (u32 x = 0; x < 2; ++x) {
            for (u32 y = 0; y < 2; ++y) {
                for (u32 z = 0; z < 2; ++z) {
                    const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f,
                                                         2.0f * y - 1.0f,
                                                         2.0f * z - 1.0f,
                                                         1.0f);
                    corners.push_back(pt / pt.w);
                }
            }
        }
        return corners;
    }

    omega::math::vec4 get_average(const std::vector<omega::math::vec4> &v) {
        omega::math::vec4 res{0.0f};
        for (const auto &a : v) {
            res += a;
        }
        return res / (f32)v.size();
    }
};

#endif // VOXEL_ENTITY_SUN_HPP
