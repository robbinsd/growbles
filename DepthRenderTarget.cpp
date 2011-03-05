#include "DepthRenderTarget.h"
#include <assert.h>

DepthRenderTarget::DepthRenderTarget() : initialized_(false)
{
}

void
DepthRenderTarget::Init(unsigned int width, unsigned int height) {
    width_ = width;
    height_ = height;

    // Initialize the texture, including filtering options
    GL_CHECK(glGenTextures(1, &textureID_));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureID_));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D,
                          0,
                          GL_DEPTH_COMPONENT,
                          width_,
                          height_,
                          0,
                          GL_DEPTH_COMPONENT,
                          GL_UNSIGNED_BYTE,
                          0));

    // Generate a framebuffer
    GL_CHECK(glGenFramebuffersEXT(1, &frameBufferID_));
    GL_CHECK(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBufferID_));

    // Attach the texture to the frame buffer
    GL_CHECK(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                       GL_DEPTH_ATTACHMENT_EXT,
                                       GL_TEXTURE_2D,
                                       textureID_,
                                       0));

    // Check the status of the FBO
    GL_CHECK(glDrawBuffer(GL_NONE));
    GL_CHECK(glReadBuffer(GL_NONE));
    assert(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT));
    GL_CHECK(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));
    GL_CHECK(glDrawBuffer(GL_BACK));

    // Mark us as initialized
    initialized_ = true;
}

DepthRenderTarget::~DepthRenderTarget() {

    // If we were initialized, release resources
    if (initialized_) {
        GL_CHECK(glDeleteFramebuffersEXT(1, &frameBufferID_));
        GL_CHECK(glDeleteRenderbuffersEXT(1, &depthBufferID_));
        GL_CHECK(glDeleteTextures(1, &textureID_));
    }
}

GLuint DepthRenderTarget::textureID() const {
    return textureID_;
}

void DepthRenderTarget::bind() {
    GL_CHECK(glPushAttrib(GL_VIEWPORT_BIT));
    GL_CHECK(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBufferID_));
    GL_CHECK(glDrawBuffer(GL_NONE));
    GL_CHECK(glViewport(0, 0, width_, height_));
}

void DepthRenderTarget::unbind() {
    GL_CHECK(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
    GL_CHECK(glDrawBuffer(GL_BACK));
    GL_CHECK(glPopAttrib());
}
