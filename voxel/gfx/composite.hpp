#ifndef VOXEL_GFX_COMPOSITE_HPP
#define VOXEL_GFX_COMPOSITE_HPP

#include "omega/gfx/gl.hpp"
#include "omega/gfx/vertex_array.hpp"
#include "omega/gfx/vertex_buffer.hpp"
#include "omega/gfx/vertex_buffer_layout.hpp"
#include "omega/util/std.hpp"

struct Composite {

    Composite() {

        const float vertices[] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            1.0f, 1.0f,

            1.0f, 1.0f,
            -1.0f, 1.0f,
            -1.0f, -1.0f
        };


        vbo = omega::util::create_uptr<omega::gfx::VertexBuffer>(vertices, sizeof(vertices));
        vao = omega::util::create_uptr<omega::gfx::VertexArray>();
        omega::gfx::VertexBufferLayout layout;
        layout.push(OMEGA_GL_FLOAT, 2);
        vao->add_buffer(*vbo, layout);
    }

    void render() {
        vao->bind();
        vbo->bind();

        omega::gfx::draw_arrays(OMEGA_GL_TRIANGLES, 0, 6);

        vbo->unbind();
        vao->unbind();
    }

    omega::util::uptr<omega::gfx::VertexBuffer> vbo = nullptr;
    omega::util::uptr<omega::gfx::VertexArray> vao = nullptr;
    
};

#endif // VOXEL_GFX_COMPOSITE_HPP
