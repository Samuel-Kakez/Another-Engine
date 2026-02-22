#pragma once

#include <glad/glad.h>

/**
 * @brief Encapsule un Framebuffer Object (FBO) OpenGL
 * @details Gère la création, destruction et opérations de base d'un FBO
 *
 */
class FrameBuffer
{
public:
    /**
     * @brief Constructeur qui crée et initialise le FBO
     *
     */
    FrameBuffer();

    /**
     * @brief Destructeur qui libère la ressource OpenGL au FBO
     *
     */
    ~FrameBuffer();

    // Empêche la copie et le déplacement
    FrameBuffer(const FrameBuffer &) = delete;
    FrameBuffer &operator=(const FrameBuffer &) = delete;
    FrameBuffer(FrameBuffer &&) = delete;
    FrameBuffer &operator=(FrameBuffer &&) = delete;

    /**
     * @brief Lie ce FBO comme framebuffer actif
     *
     */
    void Bind() const;

    /**
     * @brief Délie le FBO pour revenir au framebuffer par défaut
     *
     */
    void Unbind() const;

    /**
     * @brief Attache une texture au FBO
     *
     * @param attachmentPoint le point d'attachement
     * @param textureId ID de la texture à attacher
     */
    void AttachTexture(GLenum attachmentPoint, GLuint textureId);

    /**
     * @brief Vérifie si le framebuffer est complet et prêt à être utilisé
     *
     * @return true si le FBO est complet, false sinon
     */
    bool IsComplete() const;

    /**
     * @brief Récupère l'ID OpenGL du FBO
     *
     * @return GLuint l'ID du FBO
     */
    GLuint GetID() const { return m_id; }

private:
    GLuint m_id = 0; // ID OpenGL du FBO
};