#ifndef VOXEL_ENTITY_WATER_HPP
#define VOXEL_ENTITY_WATER_HPP

#include <array>

#include "omega/gfx/gl.hpp"
#include "omega/gfx/index_buffer.hpp"
#include "omega/gfx/shader.hpp"
#include "omega/gfx/vertex_array.hpp"
#include "omega/gfx/vertex_buffer.hpp"
#include "omega/gfx/vertex_buffer_layout.hpp"
#include "omega/math/math.hpp"
#include "omega/util/std.hpp"
#include "omega/util/time.hpp"
#include "omega/util/types.hpp"

struct Water {
    struct Vertex {
        omega::math::vec2 pos;
    };
    Water() {
        create_mesh();
    }

    void create_mesh() {
        std::array<Vertex, (depth + 1) * (width + 1)> vertices;
        u32 i = 0;
        for (u32 z = 0; z <= depth; ++z) {
            for (u32 x = 0; x <= width; ++x) {
                vertices[i].pos = omega::math::vec2(x, z);
                vertices[i].pos -=
                    omega::math::vec2(width, depth) * 0.5f; // center at 0, 0
                i++;
            }
        }
        vbo = omega::util::create_uptr<omega::gfx::VertexBuffer>(
            vertices.data(), vertices.size() * sizeof(Vertex));

        vao = omega::util::create_uptr<omega::gfx::VertexArray>();
        omega::gfx::VertexBufferLayout layout;
        layout.push(OMEGA_GL_FLOAT, 2);
        vao->add_buffer(*vbo, layout);

        const auto vertex_idx = [&](u32 x, u32 z) -> u32 {
            return z * (width + 1) + x;
        };

        std::array<u32, width * depth * 6> indices;
        i = 0;
        const auto quad_a = [&](u32 &i, u32 x, u32 z) {
            // triangle 1
            indices[i++] = vertex_idx(x, z);
            indices[i++] = vertex_idx(x + 1, z);
            indices[i++] = vertex_idx(x, z + 1);
            // triangle 2
            indices[i++] = vertex_idx(x + 1, z);
            indices[i++] = vertex_idx(x + 1, z + 1);
            indices[i++] = vertex_idx(x, z + 1);
        };
        const auto quad_b = [&](u32 &i, u32 x, u32 z) {
            // triangle 1
            indices[i++] = vertex_idx(x, z);
            indices[i++] = vertex_idx(x + 1, z + 1);
            indices[i++] = vertex_idx(x, z + 1);
            // triangle 2
            indices[i++] = vertex_idx(x, z);
            indices[i++] = vertex_idx(x + 1, z);
            indices[i++] = vertex_idx(x + 1, z + 1);
        };
        for (u32 z = 0; z < depth; ++z) {
            for (u32 x = 0; x < width; ++x) {
                // alternate between each quad to make less repetitive
                // quad A
                // |\   |
                // | \  |
                // |  \ |
                // |___\|
                // quad B
                // |   /|
                // |  / |
                // | /  |
                // |/___|

                // essentially a checkerboard pattern
                if ((i / 6) % 2 == 0) {
                    quad_a(i, x, z);
                } else {
                    quad_b(i, x, z);
                }
            }
        }
        ibo = omega::util::create_uptr<omega::gfx::IndexBuffer>(indices.data(),
                                                                indices.size());
    }

    void render() {
        vao->bind();
        ibo->bind();

        omega::gfx::draw_elements(OMEGA_GL_TRIANGLES,
                                  ibo->get_count(),
                                  OMEGA_GL_UNSIGNED_INT,
                                  nullptr);

        ibo->unbind();
        vao->unbind();
    }

    constexpr static u32 width = 300, depth = 300;

    constexpr static f32 height = 29.95f;
    omega::util::uptr<omega::gfx::VertexBuffer> vbo = nullptr;
    omega::util::uptr<omega::gfx::VertexArray> vao = nullptr;
    omega::util::uptr<omega::gfx::IndexBuffer> ibo = nullptr;

  private:
};

#endif // VOXEL_ENTITY_WATER_HPP
