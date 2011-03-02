#ifndef MATERIAL_H
#define MATERIAL_H

#include "Texture.h"
#include "Context.h"
#include "Framework.h"

/*
 * Enumeration of the different types of textures we can have.
 */
typedef enum {
    TEXTURETYPE_DIFFUSE = 0,
    TEXTURETYPE_SPECULAR,
    TEXTURETYPE_NORMAL,
    TEXTURETYPE_COUNT
} TextureType;

/*
 * Class to encapsulate the management and activation/deactivation
 * of a material (mostly for textures).
 */
class Material {

public:

    Material(Context& context);

    void InitWithMaterial(const aiMaterial* material);

    void SetEnabled(bool enabled);

    /*
     * Destroy our data.
     */
    void Destroy();

    /*
     * We let users muck with our colors if they want to.
     */

    // Ambient, Diffuse, Specular colors
    Vector mAmbient, mDiffuse, mSpecular;

    // Shininess
    GLfloat mShininess;


protected:

    void TryLoadTexture(const char* prefix, TextureType type);

    // Our global context
    Context* mContext;

    // Our array of textures. We only initialize the ones we find.
    Texture mTextures[TEXTURETYPE_COUNT];

    // Black
    Vector mBlack;
};

#endif /* MATERIAL_H */
