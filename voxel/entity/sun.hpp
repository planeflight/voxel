#ifndef VOXEL_ENTITY_SUN_HPP
#define VOXEL_ENTITY_SUN_HPP

#include "omega/scene/orthographic_camera.hpp"
#include "omega/math/math.hpp"
#include "voxel/entity/player.hpp"

struct Sun : omega::scene::OrthographicCamera {
    omega::math::vec3 direction{0.0f};
    omega::math::vec3 ambient{0.0f};
    omega::math::vec3 diffuse{0.0f};

    constexpr static float near = -100.0f, far = 100.0f;

    Sun() : omega::scene::OrthographicCamera::OrthographicCamera(near, far,
                                                                 near, far,
                                                                 near, far) {
    }

    void update_camera(const Player &player) {
        using namespace omega;

        const f32 distance = 128.0f;
        const f32 frustum_size = 256.0f;
        const math::vec2 depth_range = glm::vec2(1.0f, 256.0f);

        math::vec3 eye, center;
        eye = player.position - direction * distance;
        center = eye + direction;
        view_matrix = math::lookAt(eye, center, math::vec3(0.0f, 1.0f, 0.0f));
        const math::vec2 min{-frustum_size}, max{frustum_size};
        projection_matrix = math::ortho(min.x, max.x, min.y, max.y, depth_range.x, depth_range.y);
    }
};


#endif // VOXEL_ENTITY_SUN_HPP
