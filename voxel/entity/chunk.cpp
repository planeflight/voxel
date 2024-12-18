#include "voxel/entity/chunk.hpp"

#include "voxel/entity/block.hpp"
#include "voxel/entity/water.hpp"
#include "voxel/util/worldgen.hpp"

static void compress_vertex(const omega::math::ivec3 &pos,
                            Direction normal,
                            const omega::math::ivec2 &tex_uv,
                            Vertex &vertex) {
    uint32_t data = 0;
    // set position
    data |= (pos.x << 0) & 0xF;
    data |= (pos.y << 4) & 0xFF0;
    data |= (pos.z << 12) & 0xF000;
    // set normal
    data |= ((uint8_t)(normal) << 16) & 0x70000;
    // set tex coord x
    data |= (tex_uv.x << 19) & 0x380000;
    // set tex coord y
    data |= (tex_uv.y << 22) & 0x1C00000;
    vertex.data = data;
}

static void create_quad(size_t x,
                        size_t y,
                        size_t z,
                        int8_t type,
                        std::vector<Quad> &quads,
                        Direction direction) {
    // calculate vertex positions
    omega::math::ivec3 vertex_positions[8];
    // back
    vertex_positions[0] = omega::math::ivec3(x, y, z);     // bottom left back
    vertex_positions[1] = omega::math::ivec3(x + 1, y, z); // bottom right back
    vertex_positions[2] = omega::math::ivec3(x + 1, y + 1, z); // top right back
    vertex_positions[3] = omega::math::ivec3(x, y + 1, z);     // top left back

    // front
    vertex_positions[4] = omega::math::ivec3(x, y, z + 1); // bottom left front
    vertex_positions[5] =
        omega::math::ivec3(x + 1, y, z + 1); // bottom right front
    vertex_positions[6] =
        omega::math::ivec3(x + 1, y + 1, z + 1); // top right front
    vertex_positions[7] = omega::math::ivec3(x, y + 1, z + 1); // top left front

    // set the texture coords
    omega::math::ivec2 tex_offset =
        omega::math::vec2((float)(type % 4), (float)(type / 4));
    omega::math::ivec2 tex_coord_bl = omega::math::ivec2(0, 0) + tex_offset;
    omega::math::ivec2 tex_coord_br = omega::math::ivec2(1, 0) + tex_offset;
    omega::math::ivec2 tex_coord_tr = omega::math::ivec2(1, 1) + tex_offset;
    omega::math::ivec2 tex_coord_tl = omega::math::ivec2(0, 1) + tex_offset;

    Quad q;
    switch (direction) {
        case Direction::left: {
            compress_vertex(vertex_positions[0], direction, tex_coord_bl, q[0]);
            compress_vertex(vertex_positions[4], direction, tex_coord_br, q[1]);
            compress_vertex(vertex_positions[7], direction, tex_coord_tr, q[2]);
            compress_vertex(vertex_positions[7], direction, tex_coord_tr, q[3]);
            compress_vertex(vertex_positions[3], direction, tex_coord_tl, q[4]);
            compress_vertex(vertex_positions[0], direction, tex_coord_bl, q[5]);
            break;
        }
        case Direction::right: {
            compress_vertex(vertex_positions[5], direction, tex_coord_bl, q[0]);
            compress_vertex(vertex_positions[1], direction, tex_coord_br, q[1]);
            compress_vertex(vertex_positions[2], direction, tex_coord_tr, q[2]);
            compress_vertex(vertex_positions[2], direction, tex_coord_tr, q[3]);
            compress_vertex(vertex_positions[6], direction, tex_coord_tl, q[4]);
            compress_vertex(vertex_positions[5], direction, tex_coord_bl, q[5]);
            break;
        }
        case Direction::top: {
            compress_vertex(vertex_positions[7], direction, tex_coord_bl, q[0]);
            compress_vertex(vertex_positions[6], direction, tex_coord_br, q[1]);
            compress_vertex(vertex_positions[2], direction, tex_coord_tr, q[2]);
            compress_vertex(vertex_positions[2], direction, tex_coord_tr, q[3]);
            compress_vertex(vertex_positions[3], direction, tex_coord_tl, q[4]);
            compress_vertex(vertex_positions[7], direction, tex_coord_bl, q[5]);
            break;
        }
        case Direction::bottom: {
            compress_vertex(vertex_positions[4], direction, tex_coord_bl, q[0]);
            compress_vertex(vertex_positions[5], direction, tex_coord_br, q[1]);
            compress_vertex(vertex_positions[1], direction, tex_coord_tr, q[2]);
            compress_vertex(vertex_positions[1], direction, tex_coord_tr, q[3]);
            compress_vertex(vertex_positions[0], direction, tex_coord_tl, q[4]);
            compress_vertex(vertex_positions[4], direction, tex_coord_bl, q[5]);
            break;
        }
        case Direction::forward: {
            compress_vertex(vertex_positions[4], direction, tex_coord_bl, q[0]);
            compress_vertex(vertex_positions[5], direction, tex_coord_br, q[1]);
            compress_vertex(vertex_positions[6], direction, tex_coord_tr, q[2]);
            compress_vertex(vertex_positions[6], direction, tex_coord_tr, q[3]);
            compress_vertex(vertex_positions[7], direction, tex_coord_tl, q[4]);
            compress_vertex(vertex_positions[4], direction, tex_coord_bl, q[5]);
            break;
        }
        case Direction::backward: {
            compress_vertex(vertex_positions[0], direction, tex_coord_bl, q[0]);
            compress_vertex(vertex_positions[1], direction, tex_coord_br, q[1]);
            compress_vertex(vertex_positions[2], direction, tex_coord_tr, q[2]);
            compress_vertex(vertex_positions[2], direction, tex_coord_tr, q[3]);
            compress_vertex(vertex_positions[3], direction, tex_coord_tl, q[4]);
            compress_vertex(vertex_positions[0], direction, tex_coord_bl, q[5]);
            break;
        }
    }
    quads.push_back(q);
}

Chunk::Chunk(const omega::math::vec3 &position) : position(position) {
    vao = omega::util::create_uptr<omega::gfx::VertexArray>();

    // create the blocks using perlin noise and other algorithms
    gen_blocks();
    // generate the mesh
    load_mesh();
}

Chunk::~Chunk() {
    if (blocks == nullptr) {
        delete[] blocks;
    }
    blocks = nullptr;
}

void Chunk::render(float dt) {
    (void)dt;
    vao->bind();
    omega::gfx::draw_arrays(OMEGA_GL_TRIANGLES, 0, vbo_offset);
    vao->unbind();
}

void Chunk::init_block(size_t x, size_t y, size_t z, int8_t type) {
    static std::vector<Direction> directions_to_add;
    directions_to_add.clear();

    // get all faces that need to be created
    get_face_directions(x, y, z, directions_to_add);

    // create a face for each direction
    for (Direction direction : directions_to_add) {
        create_quad(x, y, z, type, quads_to_add, direction);
        vbo_offset += 6;
    }
}

void Chunk::remove_block(size_t x, size_t y, size_t z) {
    size_t idx = get_index(x, y, z);
    if (idx < width * height * depth) {
        blocks[idx].type = BlockType::NONE;
    }
}

void Chunk::add_block(size_t x, size_t y, size_t z, int8_t type) {
    size_t idx = get_index(x, y, z);
    if (idx < width * height * depth) {
        blocks[idx].type = (BlockType)type;
    }
}

void Chunk::update_chunk() {
    // clear quads to add
    quads_to_add.clear();
    vbo_offset = 0;

    load_mesh();
}

void Chunk::gen_blocks() {
    // initialize with empty blocks
    blocks = new Block[max_cubes];
    omega::core::assert(blocks != nullptr, "No memory available for blocks!");

    // fill with clear blocks
    size_t x, y, z;
    for (size_t i = 0; i < max_cubes; ++i) {
        get_xyz(i, x, y, z);
        blocks[i] = Block{BlockType::NONE, (u8)x, (u8)y, (u8)z};
    }

    auto *generator = WorldGen::instance();
    generator->gen(blocks, position, width, depth, height);
}

void Chunk::load_mesh() {
    // create all the necessary faces
    for (size_t i = 0; i < max_cubes; ++i) {
        Block &b = blocks[i];
        if (b.type != BlockType::NONE) {
            init_block(b.x, b.y, b.z, (i8)b.type);
        }
    }
    // send all quads to the GPU
    vbo = omega::util::create_uptr<omega::gfx::VertexBuffer>(
        quads_to_add.data(), sizeof(Quad) * quads_to_add.size());
    omega::gfx::VertexBufferLayout layout;
    layout.push(GL_INT, 1); // data
    vao->add_buffer(*vbo, layout);
}

void Chunk::get_face_directions(size_t x,
                                size_t y,
                                size_t z,
                                std::vector<Direction> &directions) {
    // handle z axis faces
    // check back face, z - 1
    if (z == 0 || !blocks[get_index(x, y, z - 1)].is_active()) {
        directions.push_back(Direction::backward);
    }
    // check front face, z + 1
    if (z == depth - 1 || !blocks[get_index(x, y, z + 1)].is_active()) {
        directions.push_back(Direction::forward);
    }

    // handle x axis faces
    // check left face, x - 1
    if (x == 0 || !blocks[get_index(x - 1, y, z)].is_active()) {
        directions.push_back(Direction::left);
    }
    // check right face, x + 1
    if (x == width - 1 || !blocks[get_index(x + 1, y, z)].is_active()) {
        directions.push_back(Direction::right);
    }

    // handle y axis faces
    // check top face, y + 1
    if (y == height - 1 || !blocks[get_index(x, y + 1, z)].is_active()) {
        directions.push_back(Direction::top);
    }
    // check bottom face, y - 1
    if (y == 0 || !blocks[get_index(x, y - 1, z)].is_active()) {
        directions.push_back(Direction::bottom);
    }
}
