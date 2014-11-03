#include "TextureShader.h"
#include "planecalib/log.h"

namespace planecalib
{

bool TextureShader::init()
{
	bool res = true;
	{
    const char *uniforms[] = { "uMVPMatrix", "uTexture" };
	int *uniformIds[] = {&mUniformMVPMatrix, &mUniformTexture};
    const int uniformsCount = sizeof(uniforms) / sizeof(uniforms[0]);

    const char *attribs[] = { "aPosCoord", "aTexCoord" };
	int *attribIds[] = {&mAttribPosCoord, &mAttribTexCoord};
    const int attribsCount = sizeof(attribs) / sizeof(attribs[0]);

    res &= mProgram.load("assets/texture_render.vert", "assets/texture_render.frag", uniforms, uniformIds, uniformsCount, attribs, attribIds,
                            attribsCount);
	}
	{
    const char *uniforms[] = { "uMVPMatrix", "uTexture", "uAlpha" };
	int *uniformIds[] = {&mUniformMVPMatrixFA, &mUniformTextureFA, &mUniformAlphaFA};
    const int uniformsCount = sizeof(uniforms) / sizeof(uniforms[0]);

    const char *attribs[] = { "aPosCoord", "aTexCoord" };
	int *attribIds[] = {&mAttribPosCoordFA, &mAttribTexCoordFA};
    const int attribsCount = sizeof(attribs) / sizeof(attribs[0]);

    res &= mProgramFA.load("assets/texture_render.vert", "assets/texture_render_fixed_alpha.frag", uniforms, uniformIds, uniformsCount, attribs, attribIds,
                            attribsCount);
	}
	return res;
}

void TextureShader::setMVPMatrix(const Eigen::Matrix4f &mvp)
{
    glUseProgram(mProgram.getId());
    glUniformMatrix4fv(mUniformMVPMatrix, 1, false, mvp.data());

    glUseProgram(mProgramFA.getId());
    glUniformMatrix4fv(mUniformMVPMatrixFA, 1, false, mvp.data());
}

void TextureShader::renderTexture(GLuint target, GLuint id, const cv::Size &imageSize,
                                    const Eigen::Vector2f &screenOrigin)
{
    const float kDepth = 1.0f;
    Eigen::Vector4f const vertices[] =
	{ Eigen::Vector4f(screenOrigin.x() + (float)imageSize.width - 1, screenOrigin.y(), kDepth, 1.0f),
	Eigen::Vector4f(screenOrigin.x(), screenOrigin.y(), kDepth, 1.0f),
	Eigen::Vector4f(screenOrigin.x() + (float)imageSize.width - 1, screenOrigin.y() + (float)imageSize.height - 1, kDepth, 1.0f),
	Eigen::Vector4f(screenOrigin.x(), screenOrigin.y() + (float)imageSize.height - 1, kDepth, 1.0f) };
    
	Eigen::Vector2f const textureCoords[] =
	{ Eigen::Vector2f(1, 0), Eigen::Vector2f(0, 0), Eigen::Vector2f(1, 1), Eigen::Vector2f(0, 1) };
    renderTexture(GL_TRIANGLE_STRIP, target, id, vertices, textureCoords, 4);
}

void TextureShader::renderTexture(GLenum mode, GLuint target, GLuint id, const Eigen::Vector4f *vertices,
                                    const Eigen::Vector2f *textureCoords, int count)
{
    assert(target == GL_TEXTURE_2D);

    glUseProgram(mProgram.getId());

    // setup uniforms
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, id);
    glUniform1i(mUniformTexture, 0);

    // drawing quad
    glVertexAttribPointer(mAttribPosCoord, 4, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(mAttribTexCoord, 2, GL_FLOAT, GL_FALSE, 0, textureCoords);
    glEnableVertexAttribArray(mAttribPosCoord);
    glEnableVertexAttribArray(mAttribTexCoord);
    glDrawArrays(mode, 0, count);
    glDisableVertexAttribArray(mAttribPosCoord);
    glDisableVertexAttribArray(mAttribTexCoord);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Fixed alpha
void TextureShader::renderTexture(GLuint target, GLuint id, const Eigen::Vector2i &imageSize,
                                    const Eigen::Vector2f &screenOrigin, float alpha)
{
    const float kDepth = 1.0f;

	//std::vector<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f>> vertices;
	std::vector<Eigen::Vector4f> vertices;
	vertices.push_back(Eigen::Vector4f(screenOrigin.x() + (float)imageSize.x() - 1, screenOrigin.y(), kDepth, 1.0f));
	vertices.push_back(Eigen::Vector4f(screenOrigin.x(), screenOrigin.y(), kDepth, 1.0f));
	vertices.push_back(Eigen::Vector4f(screenOrigin.x() + (float)imageSize.x() - 1, screenOrigin.y() + (float)imageSize.y() - 1, kDepth, 1.0f));
	vertices.push_back(Eigen::Vector4f(screenOrigin.x(), screenOrigin.y() + (float)imageSize.y() - 1, kDepth, 1.0f));
	
    Eigen::Vector2f const textureCoords[] =
    { Eigen::Vector2f(1, 0), Eigen::Vector2f(0, 0), Eigen::Vector2f(1, 1), Eigen::Vector2f(0, 1) };
    renderTexture(GL_TRIANGLE_STRIP, target, id, vertices.data(), textureCoords, 4, alpha);
}

void TextureShader::renderTexture(GLenum mode, GLuint target, GLuint id, const Eigen::Vector4f *vertices,
                                    const Eigen::Vector2f *textureCoords, int count, float alpha)
{
    assert(target == GL_TEXTURE_2D);

    glUseProgram(mProgramFA.getId());

    // setup uniforms
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, id);
    glUniform1i(mUniformTextureFA, 0);
    glUniform1f(mUniformAlphaFA, alpha);

    // drawing quad
    glVertexAttribPointer(mAttribPosCoordFA, 4, GL_FLOAT, GL_FALSE, 0, vertices);
    glVertexAttribPointer(mAttribTexCoordFA, 2, GL_FLOAT, GL_FALSE, 0, textureCoords);
    glEnableVertexAttribArray(mAttribPosCoordFA);
    glEnableVertexAttribArray(mAttribTexCoordFA);
    glDrawArrays(mode, 0, count);
    glDisableVertexAttribArray(mAttribPosCoordFA);
    glDisableVertexAttribArray(mAttribTexCoordFA);
}

}