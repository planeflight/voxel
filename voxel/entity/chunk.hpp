#ifndef VOXEL_ENTITY_CHUNK_H
#define VOXEL_ENTITY_CHUNK_H

#include "omega/core/core.hpp"
#include "omega/gfx/gfx.hpp"
#include "omega/scene/scene.hpp"
#include "omega/util/util.hpp"
#include "voxel/entity/block.hpp"

enum class Direction : uint8_t {
    left = 0,
    right = 1,
    forward = 2,
    backward = 3,
    top = 4,
    bottom = 5
};

/**
 * [0-4) -> x             2^4 = 15 + 1
 * [4-11) -> y            2^7 = 127 + 1
 * [11-15) -> z           2^4 = 15 + 1
 * [15-18) -> normal      2^3 > 6
 * [18-21) -> tex coord x 2^3 > 4 + 1
 * [21-24) -> tex coord y 2^3 > 4 + 1
 */

struct Vertex {
    uint32_t data;
};

using Quad = std::array<Vertex, 6>;

class Chunk {
  public:
    Chunk(const glm::vec3 &position);
    ~Chunk();

    void render(float dt);
    void init_block(size_t x, size_t y, size_t z, int8_t type);

    const glm::vec3 &get_position() const {
        return position;
    }
    void set_position(const glm::vec3 &position) {
        this->position = position;
    }

    constexpr static uint32_t width = 15;  // x axis
    constexpr static uint32_t depth = 15;  // z axis
    constexpr static uint32_t height = 90; // y axis
    constexpr static glm::vec3 dimens = glm::vec3(width, height, depth);
    constexpr static size_t max_cubes = width * depth * height;

    bool block_active(size_t x, size_t y, size_t z) {
        return blocks[get_index(x, y, z)].is_active();
    }

    void remove_block(size_t x, size_t y, size_t z);
    void add_block(size_t x, size_t y, size_t z, int8_t type);
    void update_chunk();

  private:
    constexpr static uint32_t num_vertices = 36; // vertices per cube

    static size_t get_index(size_t x, size_t y, size_t z) {
        return (z * width * height) + (y * width) + x;
    }

    static void get_xyz(size_t idx, size_t &x, size_t &y, size_t &z) {
        z = idx / (width * height);
        idx -= (z * width * height);
        y = idx / width;
        x = idx % width;
    }

    void gen_blocks();
    void load_mesh();

    void get_face_directions(size_t x,
                             size_t y,
                             size_t z,
                             std::vector<Direction> &directions);

    // GL render settings
    constexpr static uint32_t vertex_attr_count = 1; // data per vertex
    omega::util::uptr<omega::gfx::VertexArray> vao = nullptr;
    omega::util::uptr<omega::gfx::VertexBuffer> vbo = nullptr;
    // block data
    Block *blocks = nullptr;
    size_t vbo_offset = 0;
    glm::vec3 position{0.0f};
    std::vector<Quad> quads_to_add;
};

#endif // VOXEL_ENTITY_CHUNK_H
