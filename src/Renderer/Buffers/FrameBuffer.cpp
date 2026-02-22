#include "Renderer/Buffers/FrameBuffer.h"
#include "Debug/Logger.h"

FrameBuffer::FrameBuffer()
{
    // Crée un identifiant unique pour le FBO
    glGenFramebuffers(1, &m_id);
}

FrameBuffer::~FrameBuffer()
{
    // Le destructeur libère la ressource GPU
    if (m_id != 0)
    {
        glDeleteFramebuffers(1, &m_id);
    }
}

void FrameBuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void FrameBuffer::Unbind() const
{
    // remet le framebuffer par défaut (écran)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::AttachTexture(GLenum attachmentPoint, GLuint textureId)
{
    // Attache la texture au FBO lié actuellement
    // Le dernier paramètre "level" est 0 car on n'utilise pas de mipmap
    glFramebufferTexture(GL_FRAMEBUFFER, attachmentPoint, textureId, 0);
}

bool FrameBuffer::IsComplete() const
{
    // S'assure que le FBO est lié avant de vérifier son état
    Bind();
    bool IsComplete = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    if (!IsComplete)
    {
        LOG_ERROR("le FBO n'est pas complet.");
    }
    Unbind();
    return IsComplete;
}