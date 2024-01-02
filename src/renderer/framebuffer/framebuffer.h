#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#define RENDERER_SHADOW_RESOLUTION 1024

//buffer types
#define RENDERER_END_BUFFER 2
#define RENDERER_G_BUFFER 3
#define RENDERER_SHADOW_BUFFER 4
#define RENDERER_BLOOM_BUFFER 5

typedef struct geometryFBO {
	unsigned int id;

	unsigned int position;
	unsigned int normal;
	unsigned int albedoSpec;

	unsigned int depthBuffer;//rbo
} geometryFBO;

typedef struct shadowFBO {
	unsigned int id;

	unsigned int depthBuffer;//texture
} shadowFBO;

typedef struct endFBO {
	unsigned int id;

	unsigned int colorBuffer;
	unsigned int depthBuffer;//rbo
} endFBO;

typedef struct renderer {
	shadowFBO shadowBuffer;
	geometryFBO gBuffer;
	endFBO endBuffer;
} renderer;

renderer renderer_create(int width, int height);
void renderer_destroy(renderer cucc);

#endif