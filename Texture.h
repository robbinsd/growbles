#ifndef TEXTURE_H
#define TEXTURE_H

#include "Framework.h"
#include "Context.h"
#include <string>

class Texture {

public:

    /*
     * Constructor.
     */
    Texture() : mContext(NULL)
              , mTextureID(0)
              , mInitialized(false) {}

    /*
     * Initializes us with a texture.
     */
    void Init(Context& context, const std::string& path);

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
    Context* mContext;
    GLuint mTextureID;
    bool mInitialized;
};

#endif /* TEXTURE_H */
