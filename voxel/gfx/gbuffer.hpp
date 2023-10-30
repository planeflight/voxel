#ifndef VOXEL_GFX_GBUFFER_HPP
#define VOXEL_GFX_GBUFFER_HPP

#include "omega/util/types.hpp"
class GBuffer {
  public:
    GBuffer(u32 width, u32 height);
    ~GBuffer();

    void geometry_pass();

  private:
    u32 width = 0, height = 0;

    u32 id = 0;

    // depth, normal, color
    u32 position_buff = 0;
    u32 normal_buff = 0;
    u32 color_spec_buff = 0;
};


#endif // VOXEL_GFX_GBUFFER_HPP
