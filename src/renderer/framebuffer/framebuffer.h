#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#define RENDERER_SHADOW_RESOLUTION 4000
#define RENDERER_WIDTH 1920
#define RENDERER_HEIGHT 1080

//ha ez valtozik, akkor a final pass shadernek is kell
#define RENDERER_KAWASAKI_FBO_COUNT 4

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

typedef struct screenFBO {
	unsigned int id;

	unsigned int colorBuffer;//rgb
} screenFBO;//a legutolso framebuffer a kepernyo elott

typedef struct bloomFBO {
	unsigned int id;
	unsigned int colorBuffer;
} bloomFBO;

typedef struct renderer {
	shadowFBO shadowBuffer;
	geometryFBO gBuffer;
	ssaoFBO ssaoBuffer;
	endFBO endBuffer;
	bloomFBO bloomBuffers[RENDERER_KAWASAKI_FBO_COUNT];//tobb kell, mert Kawasaki blur lesz
	screenFBO screenBuffer;
} renderer;

renderer renderer_create(int width, int height);
void renderer_destroy(renderer cucc);

#endif