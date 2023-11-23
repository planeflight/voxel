#ifndef VOXEL_ENTITY_SUN_HPP
#define VOXEL_ENTITY_SUN_HPP

#include "omega/scene/orthographic_camera.hpp"
#include "omega/math/math.hpp"
#include "voxel/entity/player.hpp"

struct Sun : omega::scene::OrthographicCamera {
    omega::math::vec3 direction{0.0f};
    omega::math::vec3 ambient{0.0f};
    omega::math::vec3 diffuse{0.0f};

    constexpr static float near = -20.0f, far = 20.0f;

    Sun() : omega::scene::OrthographicCamera::OrthographicCamera(near, far,
                                                                 near, far,
                                                                 near, far) {
    }

    void update_camera(const Player &player) {
        using namespace omega;

        const f32 distance = 128.0f;
        const f32 frustum_size = 128.0f;
        const math::vec2 depth_range = glm::vec2(1.0f, 128.0f);

        math::vec3 eye{0.0f}, center{0.0f}, up{0.0f, 1.0f, 0.0f};
        // eye = player.position - direction;
        eye = - direction;
        // center = player.position + math::vec3(0.0f);
        center = math::vec3(0.0f);
        view_matrix = math::lookAt(eye, center, up);

        // have the projection follow the player rather than the view, but only occasionally to remove shadow movemetns
        math::vec2 min{-frustum_size}, max{frustum_size};
        min += math::vec2(player.position.x, player.position.z);
        max += math::vec2(player.position.x, player.position.z);
        projection_matrix = math::ortho(min.x, max.x, min.y, max.y, min.y, max.y);
    }

    std::vector<omega::math::vec4> get_frustrum_corners(const omega::math::mat4 &view_proj) {
        using namespace omega;
        math::mat4 inv = math::inverse(view_proj);
        std::vector<math::vec4> corners;
        for (u32 x = 0; x < 2; ++x) {
            for (u32 y = 0; y < 2; ++y) {
                for (u32 z = 0; z < 2; ++z) {
                    const glm::vec4 pt = inv * glm::vec4(2.0f * x - 1.0f, 2.0f * y - 1.0f, 2.0f * z - 1.0f, 1.0f);
                    corners.push_back(pt / pt.w);
                }
            }
        }
        return corners;
    }
};


#endif // VOXEL_ENTITY_SUN_HPP
