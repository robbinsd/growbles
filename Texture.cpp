#include "Texture.h"
#include <string>
#include <assert.h>

using std::string;
using sf::Image;

void
Texture::Init(Context& context, const string& path)
{
    // Save our parameters
    mContext = &context;

    // Read in the image
    Image image;
#ifndef NDEBUG
    bool rv =
#endif
    image.LoadFromFile(path);
    assert(rv);

    // Load it into a texture
    GL_CHECK(glGenTextures(1, &mTextureID));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, mTextureID));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, 4, image.GetWidth(), image.GetHeight(),
                          0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetPixelsPtr()));

    // Mark us as initialized
    mInitialized = true;
}

void
Texture::Destroy()
{
    if (mInitialized) {
        mContext = NULL;
        GL_CHECK(glDeleteTextures(1, &mTextureID));
        mInitialized = false;
    }
}

void
Texture::SetEnabled(bool enabled, GLenum textureUnit)
{
    // No-op if we're not initialized
    if (!mInitialized)
        return;

    // Bind either our texture or the null texture
    GL_CHECK(glActiveTexture(textureUnit));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, enabled ? mTextureID : 0));
}
