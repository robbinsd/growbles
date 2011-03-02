#ifndef TEXTURE_H
#define TEXTURE_H

#include "Framework.h"
#include <string>

class RenderContext;

class Texture {

public:

    /*
     * Constructor.
     */
    Texture() : mTextureID(0)
              , mInitialized(false) {}

    /*
     * Initializes us with a texture.
     */
    void Init(const std::string& path);

    /*
     * Frees resources.
     */
    void Destroy();

    /*
     * Sets the texture enabled/disabled.
     *
     * No-op if the texture isn't initialized.
     */
    void SetEnabled(bool enabled, GLenum textureUnit);

    /*
     * Are we initialized?
     */
    bool IsInitialized() { return mInitialized; }

protected:
    GLuint mTextureID;
    bool mInitialized;
};

#endif /* TEXTURE_H */
