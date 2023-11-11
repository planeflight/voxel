#ifndef VOXEL_GFX_GBUFFER_HPP
#define VOXEL_GFX_GBUFFER_HPP

#include "omega/util/types.hpp"

class GBuffer {
  public:
    GBuffer(u32 width, u32 height);
    ~GBuffer();

    void geometry_pass();

    void bind_geometry_fbo();
    void bind_shadow_fbo();
    void bind_geometry_textures();
    void bind_shadow_textures();

    u32 get_width() const { return width; }
    u32 get_height() const { return height; }

  private:
    u32 width = 0, height = 0;

    u32 id = 0, rbo = 0;

    // depth, normal, color
    u32 position_buff = 0;
    u32 normal_buff = 0;
    u32 color_spec_buff = 0;

    // shadows
    u32 shadow_id = 0;
    u32 light_buff = 0;
};


#endif // VOXEL_GFX_GBUFFER_HPP
