#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#define RENDERER_SHADOW_RESOLUTION renderer_getShadowResolution()
#define RENDERER_WIDTH renderer_getWidth()
#define RENDERER_HEIGHT renderer_getHeight()

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
	bloomFBO bloomBuffers[RENDERER_KAWASAKI_FBO_COUNT];//tobb kell, mert Kawasaki blur lesz (az elso csak a filterre van, feles felbontassal, a tobbi a kawasaki, ahol az elso feles felbontasu, es a tobbi felbontasa mindig az ot megelozo fele)
	screenFBO screenBuffer;
} renderer;

int renderer_getWidth();
int renderer_getHeight();
int renderer_getShadowResolution();
void renderer_setWidth(int _width);
void renderer_setHeight(int _height);
void renderer_setShadowResolution(int _res);

renderer renderer_create(int width, int height);
void renderer_destroy(renderer cucc);

#endif