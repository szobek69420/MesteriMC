#include "hand_renderer.h"

#include <glad/glad.h>
#include <stdlib.h>

#include "../../glm2/mat4.h"
#include "../../glm2/mat3.h"
#include "../../shader/shader.h"

#include "../../world/blocks/blocks.h"

#include "../../texture_handler/texture_handler.h"

struct handRenderer {
	shader program;
	unsigned int blockVao, blockVbo, blockEbo;
	unsigned int itemVao, itemVbo, itemEbo;
};

static unsigned int blockIndices[36];
static float blockVertexPositions[72];
static const int blockVertexCount = 24;
static const int blockVertexAttributeCount = 11;

handRenderer* handRenderer_create()
{
	handRenderer* hr = malloc(sizeof(handRenderer));

	hr->program = shader_import(
		"../assets/shaders/renderer/deferred_geometry/shader_deferred_geometry_unpacked.vag",
		"../assets/shaders/renderer/deferred_geometry/shader_deferred_geometry.fag",
		NULL
	);

	glUseProgram(hr->program.id);
	glUniform1i(glGetUniformLocation(hr->program.id, "texture_albedo"), 0);
	glUniform1i(glGetUniformLocation(hr->program.id, "texture_normal"), 1);
	glUniform1i(glGetUniformLocation(hr->program.id, "texture_specular"), 2);
	glUseProgram(0);

	//block
	float* blockVertexData = malloc(blockVertexCount * blockVertexAttributeCount * sizeof(float));
	float x, y, z;
	int currentIndex = 0;
	for (int side = 0; side < 6; side++)
	{
		for (int indexInSide = 0; indexInSide < 4; indexInSide++, currentIndex += blockVertexAttributeCount)
		{
			blockVertexData[currentIndex] = blockVertexPositions[12 * side + 3*indexInSide];
			blockVertexData[currentIndex+1] = blockVertexPositions[12 * side + 3*indexInSide+1];
			blockVertexData[currentIndex+2] = blockVertexPositions[12 * side + 3*indexInSide+2];

			blocks_getUV(BLOCK_SUS, side, indexInSide, &x, &y);
			blockVertexData[currentIndex + 3] = x;
			blockVertexData[currentIndex + 4] = y;

			blocks_getVertexNormal(side, &x, &y, &z);
			blockVertexData[currentIndex + 5] = x;
			blockVertexData[currentIndex + 6] = y;
			blockVertexData[currentIndex + 7] = z;

			blocks_getVertexTangent(side, &x, &y, &z);
			blockVertexData[currentIndex + 8] = x;
			blockVertexData[currentIndex + 9] = y;
			blockVertexData[currentIndex + 10] = z;
		}
	}

	glGenVertexArrays(1, &hr->blockVao);
	glBindVertexArray(hr->blockVao);


	glGenBuffers(1, &hr->blockVbo);
	glBindBuffer(GL_ARRAY_BUFFER, hr->blockVbo);
	glBufferData(GL_ARRAY_BUFFER, blockVertexCount * blockVertexAttributeCount * sizeof(float), blockVertexData, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, blockVertexAttributeCount * sizeof(float), NULL);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, blockVertexAttributeCount * sizeof(float), 3 * sizeof(float));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, blockVertexAttributeCount * sizeof(float), 5 * sizeof(float));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, blockVertexAttributeCount * sizeof(float), 8 * sizeof(float));

	glEnableVertexAttribArray(0);//position
	glEnableVertexAttribArray(1);//uv
	glEnableVertexAttribArray(2);//normal
	glEnableVertexAttribArray(3);//tangent

	
	glGenBuffers(1, &hr->blockEbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hr->blockEbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(blockIndices), blockIndices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	free(blockVertexData);

	//item
	hr->itemVao = 0;
	hr->itemVbo = 0;
	hr->itemEbo = 0;



	return hr;
}

void handRenderer_destroy(handRenderer* hr)
{
	shader_delete(&hr->program);

	glDeleteVertexArrays(1, &hr->blockVao);
	glDeleteBuffers(1, &hr->blockVbo);
	glDeleteBuffers(1, &hr->blockEbo);

	free(hr);
}

void handRenderer_renderBlock(handRenderer* hr, mat4* model, mat4* model_normal)
{
	glUseProgram(hr->program.id);
	glUniformMatrix4fv(glGetUniformLocation(hr->program.id, "model"), 1, GL_FALSE, model->data);
	glUniformMatrix3fv(glGetUniformLocation(hr->program.id, "model_normal"), 1, GL_FALSE, model_normal->data);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_ATLAS_ALBEDO));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_ATLAS_NORMAL));
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textureHandler_getTexture(TEXTURE_ATLAS_SPECULAR_REDUCED));

	glBindVertexArray(hr->blockVao);

	glDrawElements(GL_TRIANGLES, sizeof(blockIndices) / sizeof(blockIndices[0]), GL_UNSIGNED_INT, NULL);

	glBindVertexArray(0);
	glUseProgram(0);
}

void handRenderer_renderItem(handRenderer* hr, mat4* model)
{

}

void handRenderer_setMatrices(handRenderer* hr, mat4* proj, mat4* view, mat3* view_normal)
{
	glUseProgram(hr->program.id);

	glUniformMatrix4fv(glGetUniformLocation(hr->program.id, "projection"), 1, GL_FALSE, proj->data);
	glUniformMatrix4fv(glGetUniformLocation(hr->program.id, "view"), 1, GL_FALSE, view->data);
	glUniformMatrix3fv(glGetUniformLocation(hr->program.id, "view_normal"), 1, GL_FALSE, view_normal->data);
}

void handRenderer_setBlock(handRenderer* hr, int blockType)
{
	float* blockVertexData = malloc(blockVertexCount * blockVertexAttributeCount * sizeof(float));
	float x, y, z;
	int currentIndex = 0;
	for (int side = 0; side < 6; side++)
	{
		for (int indexInSide = 0; indexInSide < 4; indexInSide++, currentIndex += blockVertexAttributeCount)
		{
			blockVertexData[currentIndex] = blockVertexPositions[12 * side + 3 * indexInSide];
			blockVertexData[currentIndex + 1] = blockVertexPositions[12 * side + 3 * indexInSide + 1];
			blockVertexData[currentIndex + 2] = blockVertexPositions[12 * side + 3 * indexInSide + 2];

			blocks_getUV(blockType, side, indexInSide, &x, &y);
			blockVertexData[currentIndex + 3] = x;
			blockVertexData[currentIndex + 4] = y;

			blocks_getVertexNormal(side, &x, &y, &z);
			blockVertexData[currentIndex + 5] = x;
			blockVertexData[currentIndex + 6] = y;
			blockVertexData[currentIndex + 7] = z;

			blocks_getVertexTangent(side, &x, &y, &z);
			blockVertexData[currentIndex + 8] = x;
			blockVertexData[currentIndex + 9] = y;
			blockVertexData[currentIndex + 10] = z;
		}
	}

	glBindVertexArray(hr->blockVao);
	glBindBuffer(GL_ARRAY_BUFFER, hr->blockVbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, blockVertexCount * blockVertexAttributeCount * sizeof(float), blockVertexData);
	glBindVertexArray(0);

	free(blockVertexData);
}

void handRenderer_setItem(handRenderer* hr, int itemType)
{

}


static unsigned int blockIndices[] = {
	0, 2, 1,	3, 2, 0,
	4, 6, 5,	7, 6, 4,
	8, 10, 9,	11, 10, 8,
	12, 14, 13,	15, 14, 12,
	16, 18, 17,	19, 18, 16,
	20, 22, 21,	23, 22, 20
};

static float blockVertexPositions[] = {
	-0.5f,-0.5f,0.5f,
	0.5f,-0.5f,0.5f,
	0.5f,0.5f,0.5f,
	-0.5f,0.5f,0.5f,

	0.5f,-0.5f,0.5f,
	0.5f,-0.5f,-0.5f,
	0.5f,0.5f,-0.5f,
	0.5f,0.5f,0.5f,

	0.5f,-0.5f,-0.5f,
	-0.5f,-0.5f,-0.5f,
	-0.5f,0.5f,-0.5f,
	0.5f,0.5f,-0.5f,

	-0.5f,-0.5f,-0.5f,
	-0.5f,-0.5f,0.5f,
	-0.5f,0.5f,0.5f,
	-0.5f,0.5f,-0.5f,

	-0.5f,0.5f,0.5f,
	0.5f,0.5f,0.5f,
	0.5f,0.5f,-0.5f,
	-0.5f,0.5f,-0.5f,

	-0.5f,-0.5f,-0.5f,
	0.5f,-0.5f,-0.5f,
	0.5f,-0.5f,0.5f,
	-0.5f,-0.5f,0.5f
};