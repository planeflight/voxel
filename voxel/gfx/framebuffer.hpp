#ifndef VOXEL_GFX_FRAMEBUFFER_HPP
#define VOXEL_GFX_FRAMEBUFFER_HPP

#include "omega/gfx/gfx.hpp"
#include <vector>

enum class FrameBufferTextureFormat : i32 {
    RGBA = GL_RGBA,
    RGBA_32F = GL_RGBA32F
};

enum class TextureParams : i32 {
    LINEAR = GL_LINEAR,
    NEAREST = GL_NEAREST,
    REPEAT = GL_REPEAT
};

struct FrameBufferAttachment {
    u32 id = 0;
    FrameBufferTextureFormat internal_fmt = FrameBufferTextureFormat::RGBA;
    FrameBufferTextureFormat external_fmt = FrameBufferTextureFormat::RGBA;
    
    TextureParams min_filter = TextureParams::NEAREST,
        mag_filter = TextureParams::NEAREST;

    void init(u32 width, u32 height) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(
            GL_TEXTURE_2D, 0, (i32) internal_fmt,
            width, height, 0, (i32) external_fmt, GL_FLOAT, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (i32) min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (i32) mag_filter);
    }
};

class FrameBuffer {
  public:
    FrameBuffer(u32 width, u32 height, std::vector<FrameBufferAttachment> attachments) : width(width), height(height), attachments(attachments) {
        glGenFramebuffers(1, &id);
        glBindFramebuffer(GL_FRAMEBUFFER, id);

        u32 i = 0;
        u32 color_attachment[this->attachments.size()];
        for (auto &att : this->attachments) {
            att.init(width, height);
            color_attachment[i] = GL_COLOR_ATTACHMENT0 + i;
            glFramebufferTexture2D(GL_FRAMEBUFFER, color_attachment[i], GL_TEXTURE_2D, att.id, 0);
            ++i;
        }
        glDrawBuffers(this->attachments.size(), color_attachment);
        // attach render buffer object as depth buffer
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER)
            != GL_FRAMEBUFFER_COMPLETE) {
            omega::util::warn("Framebuffer is not complete!");
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~FrameBuffer() {
        // need to add cleanup functionality
    }

    std::vector<FrameBufferAttachment> attachments;
    u32 id = 0;
    u32 rbo = 0;
    u32 width = 0, height = 0;
};

#endif // VOXEL_GFX_FRAMEBUFFER_HPP
