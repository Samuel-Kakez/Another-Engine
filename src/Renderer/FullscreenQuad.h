#pragma once

#include <glad/glad.h>

/// @brief Singleton pour dessiner un quad couvrant tout l'écran
/// @details Utilisé par toutes les passes de post-processing
class FullscreenQuad
{
public:
    /// @brief Retourne l'instance unique
    /// @return
    static FullscreenQuad &Instance();

    /// @brief  Dessine le quad (il faut bind un shader avant)
    void Draw() const;

    FullscreenQuad(const FullscreenQuad &) = delete;
    FullscreenQuad &operator=(const FullscreenQuad &) = delete;

private:
    FullscreenQuad();
    ~FullscreenQuad();

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
};