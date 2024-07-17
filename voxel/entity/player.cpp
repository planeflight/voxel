#include "voxel/entity/player.hpp"

using namespace omega;

Player::Player(const math::vec3 &pos, const math::vec3 &dimens) :
    omega::scene::PerspectiveCamera::PerspectiveCamera(pos),
    dimens(dimens) {
}

void Player::update(f32 dt, f32 gravity) {
    // velocity += math::vec3(0.0f, gravity, 0.0f) * 0.5f * dt;
    position += velocity * dt;
    // velocity += math::vec3(0.0f, gravity, 0.0f) * 0.5f * dt;
}

void Player::handle_collisions(f32 dt, Chunk *chunk) {
    (void)dt;

    // get colliders
    i32 x = (i32) math::floor(position.x) % (i32) chunk->dimens.x;
    i32 y = (i32) math::floor(position.x) % (i32) chunk->dimens.y;
    i32 z = (i32) math::floor(position.z) % (i32) chunk->dimens.z;
    // if the y is beyond the dimensions, there cant be collision
    if (y >= chunk->dimens.y) { return; }

    // check the 9 blocks around it
    // for (i32 z_pos = math::min(0, z))
    // util::info("{} {} {}", x, y, z);
    if (position.y < 1.0f + dimens.y / 2.0f) {
        position.y = 1.0f + dimens.y / 2.0f;
    }
}

