#ifndef VOXEL_ENTITY_PLAYER_HPP
#define VOXEL_ENTITY_PLAYER_HPP

#include "omega/math/math.hpp"
#include "omega/scene/perspective_camera.hpp"
#include "voxel/entity/chunk.hpp"

class Player : public omega::scene::PerspectiveCamera {
  public:
    Player(const omega::math::vec3 &pos, const omega::math::vec3 &dimens);
    void update(f32 dt, f32 gravity);

    void handle_collisions(f32 dt, Chunk *chunk);

    omega::math::vec3 velocity{0.0f};

  private:
    omega::math::vec3 dimens{1.0f};
};

#endif // VOXEL_ENTITY_PLAYER_HPP
