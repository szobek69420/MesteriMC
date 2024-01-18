#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#define RENDERER_SHADOW_RESOLUTION 2000
#define RENDERER_WIDTH 1920
#define RENDERER_HEIGHT 1080

//buffer types
#define RENDERER_END_BUFFER 2
#define RENDERER_G_BUFFER 3
#define RENDERER_SHADOW_BUFFER 4
#define RENDERER_BLOOM_BUFFER 5

typedef struct geometryFBO {
	unsigned int id;

	unsigned int normal;
	unsigned int albedoSpec;

	unsigned int depthBuffer;//texture
} geometryFBO;

typedef struct ssaoFBO {
	unsigned int idColor;
	unsigned int idBlur;

	unsigned int colorBuffer;
	unsigned int colorBufferBlur;
} ssaoFBO;

typedef struct shadowFBO {
	unsigned int id;

	unsigned int depthBuffer;//texture
} shadowFBO;

typedef struct endFBO {
	unsigned int id;

	unsigned int colorBuffer;
	unsigned int depthBuffer;//texture
} endFBO;

typedef struct renderer {
	shadowFBO shadowBuffer;
	geometryFBO gBuffer;
	ssaoFBO ssaoBuffer;
	endFBO endBuffer;
} renderer;

renderer renderer_create(int width, int height);
void renderer_destroy(renderer cucc);

#endif