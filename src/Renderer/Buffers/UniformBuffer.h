#pragma once

#include <glad/glad.h>

/**
 * @brief Encapsule un Uniform Buffer Object (UBO) OpenGL.
 * @details Gère la création, la destruction, la mise à jour d'un buffer de données partagée pour les shaders,
 * en suivant le principe RAAI
 *
 */
class UniformBuffer
{
public:
    /**
     * @brief Constructeur. Crée le buffer et alloue sa mémoire.
     *
     * @param size la taille en octets du buffer à allouer
     * @param bindingPoint le point de liaison global auquel ce buffer sera associé
     */
    UniformBuffer(size_t size, GLuint bindingPoint);

    /**
     * @brief Destructeur. Libère la ressource OpenGL.
     *
     */
    ~UniformBuffer();

    // Rendre la classe non-copiable pour une gestion saine des ressources.
    UniformBuffer(const UniformBuffer &) = delete;
    UniformBuffer &operator=(const UniformBuffer &) = delete;

    /**
     * @brief Met à jour une partie ou la totalité des données du buffer.
     *
     * @param data Pointeur vers les nouvelles données
     * @param size La taille en octets des données à envoyer
     * @param offset L'offset en octets à partir duquel commencer à écrire dans le buffer
     */
    void SetData(const void *data, size_t size, size_t offset = 0);

private:
    GLuint m_id = 0; // L'ID OpenGL du buffer
};