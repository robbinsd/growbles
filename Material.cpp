#include "Material.h"
#include "RenderContext.h"
#include <string>
#include <fstream>
#include <assert.h>

using std::string;
using std::ifstream;

#define SHININESS_DEFAULT 127 // From Piazzza

Material::Material(RenderContext& context) : mShininess(SHININESS_DEFAULT)
                                           , mContext(&context)
{
}

void
Material::InitWithMaterial(const aiMaterial* material)
{
    // We don't handle multiple textures for a given type
    assert(material->GetTextureCount(aiTextureType_DIFFUSE) <= 1);

    // Load the texture prefix
    aiString prefix;
    material->GetTexture(aiTextureType_DIFFUSE, 0, &prefix);

    // Try loading each texture. If it's not there, we just don't initialize
    // that texture object.
    for (unsigned i = 0; i < TEXTURETYPE_COUNT; ++i)
        TryLoadTexture(prefix.data, (TextureType) i);

    // Load Material Properties
    aiColor3D color;

    // Ambient
    material->Get(AI_MATKEY_COLOR_AMBIENT, color);
    mAmbient.Set(color.r, color.g, color.b, 1.0);

    // Diffuse
    material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    mDiffuse.Set(color.r, color.g, color.b, 1.0);

    // Specular
    material->Get(AI_MATKEY_COLOR_SPECULAR, color);
    mSpecular.Set(color.r, color.g, color.b, 1.0);

    // Shininess
    material->Get(AI_MATKEY_SHININESS, mShininess);
}

void
Material::Destroy()
{
    // Destroy each of our textures
    for (unsigned i = 0; i < TEXTURETYPE_COUNT; ++i)
        mTextures[i].Destroy();

    // Null out our context
    mContext = NULL;
}

void
Material::SetEnabled(bool enabled)
{
    // Ambient
    SET_UNIFORMV(mContext, 3fv, "Ka", enabled ? mAmbient.Get() : mBlack.Get());

    // Diffuse
    SET_UNIFORMV(mContext, 3fv, "Kd", enabled ? mDiffuse.Get() : mBlack.Get());

    // Specular
    SET_UNIFORMV(mContext, 3fv, "Ks", enabled ? mSpecular.Get() : mBlack.Get());

    // Shininess
    SET_UNIFORM(mContext, 1f, "alpha", enabled ? mShininess : SHININESS_DEFAULT);

    // Textures
    mTextures[TEXTURETYPE_DIFFUSE].SetEnabled(enabled, DIFFUSE_TEXTURE_UNIT);
    mTextures[TEXTURETYPE_SPECULAR].SetEnabled(enabled, SPECULAR_TEXTURE_UNIT);
    mTextures[TEXTURETYPE_NORMAL].SetEnabled(enabled, NORMAL_TEXTURE_UNIT);

    // Are we doing normal mapping?
    SET_UNIFORM(mContext, 1i, "mapNormals",
                enabled && mTextures[TEXTURETYPE_NORMAL].IsInitialized() ? 1 : 0);

}

static const char* sSuffixes[] = {"_d.jpg", "_s.jpg", "_n.jpg"};
static const char* sPrefix = "scenefiles/";

void
Material::TryLoadTexture(const char* prefix, TextureType type)
{
    // Get the full path, concatenated with the suffix
    string fullPath = string(sPrefix) + string(prefix) +
                      string(sSuffixes[type]);

    // If the file exists, initialize the appropriate texture
    ifstream file(fullPath.c_str(), ifstream::in);
    if (file)
        mTextures[type].Init(fullPath);
}
