#include <glad/glad.h>
#include <stdio.h>

#include "framebuffer.h"

shadowFBO renderer_createShadowFBO(int width, int height);
void renderer_destroyShadowFBO(shadowFBO shadowBuffer);

geometryFBO renderer_createGeometryFBO(int width, int height);
void renderer_destroyGeometryFBO(geometryFBO gBuffer);

endFBO renderer_createEndFBO(int width, int height);
void renderer_destroyEndFBO(endFBO endBuffer);



renderer renderer_create(int width, int height)
{
    renderer rendor;
    rendor.shadowBuffer = renderer_createShadowFBO(RENDERER_SHADOW_RESOLUTION,RENDERER_SHADOW_RESOLUTION);
    rendor.gBuffer = renderer_createGeometryFBO(width, height);
    rendor.endBuffer = renderer_createEndFBO(width, height);
    return rendor;
}

void renderer_destroy(renderer cucc)
{
    renderer_destroyShadowFBO(cucc.shadowBuffer);
    renderer_destroyGeometryFBO(cucc.gBuffer);
    renderer_destroyEndFBO(cucc.endBuffer);
}


shadowFBO renderer_createShadowFBO(int width, int height)
{
    shadowFBO fb;

    glGenFramebuffers(1, &(fb.id));
    glBindFramebuffer(GL_FRAMEBUFFER, fb.id);

    glGenTextures(1, &fb.depthBuffer);
    glBindTexture(GL_TEXTURE_2D, fb.depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, fb.depthBuffer, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("something is fucked with the shadow framebuffer");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fb;
}

void renderer_destroyShadowFBO(shadowFBO shadowBuffer)
{
    glDeleteTextures(1, &shadowBuffer.depthBuffer);
    glDeleteFramebuffers(1, &shadowBuffer.id);
}

geometryFBO renderer_createGeometryFBO(int width, int height)
{
    geometryFBO gBuffer;

    glGenFramebuffers(1, &gBuffer.id);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.id);

    // - normal color buffer
    glGenTextures(1, &gBuffer.normal);
    glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.normal, 0);

    // - color + specular color buffer
    glGenTextures(1, &gBuffer.albedoSpec);
    glBindTexture(GL_TEXTURE_2D, gBuffer.albedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.albedoSpec, 0);

    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    //depth
    glGenTextures(1, &gBuffer.depthBuffer);
    glBindTexture(GL_TEXTURE_2D, gBuffer.depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gBuffer.depthBuffer, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("big problem with the geometry framebuffer");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return gBuffer;
}

void renderer_destroyGeometryFBO(geometryFBO gBuffer)
{
    //glDeleteTextures(2,&gBuffer.normal);
    glDeleteTextures(1, &gBuffer.normal);
    glDeleteTextures(1, &gBuffer.albedoSpec);

    glDeleteTextures(1, &gBuffer.depthBuffer);

    glDeleteFramebuffers(1, &gBuffer.id);
}

endFBO renderer_createEndFBO(int width, int height)
{
    endFBO endBuffer;

    glGenFramebuffers(1, &(endBuffer.id));
    glBindFramebuffer(GL_FRAMEBUFFER, endBuffer.id);

    glGenTextures(1, &(endBuffer.colorBuffer));
    glBindTexture(GL_TEXTURE_2D, endBuffer.colorBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, endBuffer.colorBuffer, 0);

    /*glGenRenderbuffers(1, &(endBuffer.depthBuffer));
    glBindRenderbuffer(GL_RENDERBUFFER, endBuffer.depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, endBuffer.depthBuffer);*/

    glGenTextures(1, &endBuffer.depthBuffer);
    glBindTexture(GL_TEXTURE_2D, endBuffer.depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, endBuffer.depthBuffer, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("something is fucked with the framebuffer");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return endBuffer;
}

void renderer_destroyEndFBO(endFBO endBuffer)
{
    glDeleteTextures(1, &endBuffer.colorBuffer);
    //glDeleteRenderbuffers(1, &endBuffer.depthBuffer);
    glDeleteTextures(1, &endBuffer.depthBuffer);
    glDeleteFramebuffers(1, &endBuffer.id);
}