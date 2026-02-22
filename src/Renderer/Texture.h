#pragma once

#include <glad/glad.h>
#include <string>

struct Texture
{
    GLuint ID = 0;
    int Width = 0;
    int Height = 0;

    Texture() = default;
    ~Texture();

    // Éviter les copies accidentelles de texture
    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;
    Texture(Texture &&) = delete;
    Texture &operator=(Texture &&) = delete;
};