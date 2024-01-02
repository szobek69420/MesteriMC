#include <glad/glad.h>
#include <stdio.h>

#include "framebuffer.h"

shadowFBO renderer_createShadowFBO(int width, int height);

renderer renderer_create();
renderer renderer_destroy(renderer cucc);


shadowFBO renderer_createShadowFBO(int width, int height)
{
    shadowFBO fb;

    glGenFramebuffers(1, &(fb.id));
    glBindFramebuffer(GL_FRAMEBUFFER, fb.id);

    glGenTextures(1, &fb.depthBuffer);
    glBindTexture(GL_TEXTURE_2D, fb.depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
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

geometryFBO renderer_createGeometryFBO(int width, int height)
{
    geometryFBO gBuffer;

    glGenFramebuffers(1, &gBuffer.id);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.id);

    // - position color buffer
    glGenTextures(1, &gBuffer.position);
    glBindTexture(GL_TEXTURE_2D, gBuffer.position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer.position, 0);

    // - normal color buffer
    glGenTextures(1, &gBuffer.normal);
    glBindTexture(GL_TEXTURE_2D, gBuffer.normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gBuffer.normal, 0);

    // - color + specular color buffer
    glGenTextures(1, &gBuffer.albedoSpec);
    glBindTexture(GL_TEXTURE_2D, gBuffer.albedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gBuffer.albedoSpec, 0);

    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    //depth
    glGenRenderbuffers(1, &(gBuffer.depthBuffer));
    glBindRenderbuffer(GL_RENDERBUFFER, gBuffer.depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gBuffer.depthBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("something is fucked with the framebuffer");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return gBuffer;
}

endFBO renderer_createEndFBO(int width, int height)
{
    endFBO endBuffer;

    glGenFramebuffers(1, &(endBuffer.id));
    glBindFramebuffer(GL_FRAMEBUFFER, endBuffer.id);

    glGenTextures(1, &(endBuffer.colorBuffer));
    glBindTexture(GL_TEXTURE_2D, endBuffer.colorBuffer);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);//itt elég lenne a GL_RGB16F, de aszem az nem minden videjókárgyával kompatibilis

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, endBuffer.colorBuffer, 0);

    glGenRenderbuffers(1, &(endBuffer.depthBuffer));
    glBindRenderbuffer(GL_RENDERBUFFER, endBuffer.depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, endBuffer.depthBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        printf("something is fucked with the framebuffer");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return endBuffer;
}