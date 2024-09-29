#ifndef VOXEL_ENTITY_PLAYER_HPP
#define VOXEL_ENTITY_PLAYER_HPP

#include "omega/math/math.hpp"
#include "omega/scene/perspective_camera.hpp"
#include "voxel/entity/chunk.hpp"

class Player : public omega::scene::PerspectiveCamera {
  public:
    Player(const omega::math::vec3 &pos, const omega::math::vec3 &dimens);
    void update(f32 dt, f32 gravity);

    void move(f32 dt, f32 gravity, const omega::math::vec3 &dir, Chunk *chunk);

    void handle_collisions(Chunk *chunk);

    omega::math::vec3 velocity{0.0f};
    constexpr static f32 speed = 30.0f;

  private:
    omega::math::vec3 dimens{1.0f};
};

#endif // VOXEL_ENTITY_PLAYER_HPP
