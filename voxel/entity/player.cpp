#include "voxel/entity/player.hpp"

#include "voxel/util/aabb.hpp"

using namespace omega;

Player::Player(const math::vec3 &pos, const math::vec3 &dimens)
    : PerspectiveCamera(pos), dimens(dimens) {}

void Player::update(f32 dt, f32 gravity) {
    // velocity += math::vec3(0.0f, gravity, 0.0f) * 0.5f * dt;
    position += velocity * dt;
    // velocity += math::vec3(0.0f, gravity, 0.0f) * 0.5f * dt;
}

f32 move_axis(u32 i, const omega::math::vec3 &move) {
    // auto colliders = get_colliders();
    // for (const auto &c : colliders) {
    //     c.x
    // }
    return 1.0f;
}

void Player::move(f32 dt, f32 gravity, const math::vec3 &dir, Chunk *chunk) {
    velocity.x = dir.x * speed;
    velocity.y = dir.y * speed;
    // velocity.y += -gravity * dt;
    velocity.z = dir.z * speed;

    math::vec3 to_move = velocity * dt;
    // calculate the possible movement on each axis
    for (u32 i = 0; i < 3; ++i) {}
    f32 x = move_axis(0, to_move);
    f32 y = move_axis(1, to_move);
    f32 z = move_axis(2, to_move);

    // move by the time given by the earliest collision
    f32 min = math::min(x, y);
    min = math::min(min, z);

    // position += velocity * min;
    position += to_move;

    // collisions
    if (chunk != nullptr) {
        // handle_collisions(chunk);
    }
}

void Player::handle_collisions(Chunk *chunk) {
    math::vec3 p_local = position - chunk->dimens * chunk->get_position();
    // create min/max AABB
    math::uvec3 min_local(0), max_local(0);
    min_local = math::floor(p_local - dimens * 0.5f);
    max_local = math::ceil(p_local + dimens * 0.5f);

    // clamp the values
    min_local = math::max(math::uvec3(0), min_local);
    max_local = math::min((math::uvec3)Chunk::dimens - 1u, max_local);
    for (u32 z = min_local.z; z <= max_local.z; ++z) {
        for (u32 y = min_local.y; z <= max_local.y; ++y) {
            for (u32 x = min_local.x; z <= max_local.x; ++x) {
                auto active = chunk->block_active(x, y, z);
                if (!active) continue;
                // there must be a collision here
                AABBf collider = AABBf::from_min({x, y, z}, {1.0f, 1.0f, 1.0f});
                AABBf player(position, dimens);
                if (player.intersect(collider)) {
                    util::info("collision {} {} {}", x, y, z);
                }
                // resolve_collision(player_box, box);
            }
        }
    }
}
