#include "gbuffer.hpp"

#include "omega/gfx/gfx.hpp"

GBuffer::GBuffer(u32 width, u32 height) : width(width), height(height) { 
    glGenFramebuffers(1, &id);
    glBindFramebuffer(GL_FRAMEBUFFER, id);

    // position color buffer
    glGenTextures(1, &position_buff);
    glBindTexture(GL_TEXTURE_2D, position_buff);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA32F,
        width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D, position_buff, 0);

    // normal color buffer
    glGenTextures(1, &normal_buff);
    glBindTexture(GL_TEXTURE_2D, normal_buff);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA32F,
        width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
        GL_TEXTURE_2D, normal_buff, 0);

    // color + specular color buffer
    glGenTextures(1, &color_spec_buff);
    glBindTexture(GL_TEXTURE_2D, color_spec_buff);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA,
        width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
        GL_TEXTURE_2D, color_spec_buff, 0);

    // add attachments
    u32 attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    // attach render buffer object as depth buffer
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);
    // check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        omega::util::warn("Framebuffer is not complete!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &shadow_id);
    glGenTextures(1, &light_buff);
    glBindTexture(GL_TEXTURE_2D, light_buff);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                 width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);  

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light_buff, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GBuffer::~GBuffer() {
    glDeleteTextures(1, &position_buff);
    glDeleteTextures(1, &normal_buff);
    glDeleteTextures(1, &color_spec_buff);

    glDeleteFramebuffers(1, &id);
    glDeleteRenderbuffers(1, &rbo);

    glDeleteTextures(1, &light_buff);

    glDeleteFramebuffers(1, &shadow_id);
}

void GBuffer::bind_geometry_fbo() {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void GBuffer::bind_shadow_fbo() {
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_id);
}

void GBuffer::bind_geometry_textures() {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, position_buff);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normal_buff);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, color_spec_buff);
}

void GBuffer::bind_shadow_textures() {
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, light_buff);
}

