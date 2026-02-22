#include "texture.h"

Texture::~Texture()
{
    if (ID != 0)
    {
        glDeleteTextures(1, &ID);
    }
}