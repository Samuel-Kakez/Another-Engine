#include "Renderer/Buffers/UniformBuffer.h"

UniformBuffer::UniformBuffer(size_t size, GLuint bindingPoint)
{

    // 1. Créé l'identifiant pour notre UBO.
    glGenBuffers(1, &m_id);

    // 2. On le lie pour le configurer
    glBindBuffer(GL_UNIFORM_BUFFER, m_id);

    // 3. On alloue la mémoire pour le buffer sur le GPU
    // GL_DYNAMIC_DRAW est une indication pour OpenGL qu'on va mettre à jour ces donées fréquemment (chaque frame)
    glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);

    // 4. On délie le bufer pour l'instant
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // 5. On associe notre buffer à un "binding point global"
    // Les shaders qui se lieront au même point aurant accès à ce buffer
    // C'est le lien entre C++ et OpenGL
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, m_id);
}

UniformBuffer::~UniformBuffer()
{
    // Le destructeur s'assure que la mémoire sur le GPU est libérée
    glDeleteBuffers(1, &m_id);
}

void UniformBuffer::SetData(const void *data, size_t size, size_t offset)
{
    // On lie le buffer pour écrire dedans
    glBindBuffer(GL_UNIFORM_BUFFER, m_id);

    // On envoie les données au GPU. glBufferSubData est plus efficace que glBufferData pour mettre à jour un buffer existant

    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);

    // On délie le buffer une fois l'opération terminée
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}