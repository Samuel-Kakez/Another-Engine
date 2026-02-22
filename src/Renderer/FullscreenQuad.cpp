#include "Renderer/FullscreenQuad.h"

FullscreenQuad &FullscreenQuad::Instance()
{
    static FullscreenQuad instance;
    return instance;
}

FullscreenQuad::FullscreenQuad()
{

    // Triangle qui couvre tout l'écran (plus efficace qu'un quad)
    // Utilise un seul triangle surdimensionné
    float vertices[] = {
        // positions (x, y)     UV (u, v)
        -1.0f, -1.0f, 0.0f, 0.0f,
        3.0f, -1.0f, 2.0f, 0.0f,
        -1.0f, 3.0f, 0.0f, 2.0f};

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

    // UV
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

    glBindVertexArray(0);
}

FullscreenQuad::~FullscreenQuad()
{
    if (m_vao != 0)
    {
        glDeleteVertexArrays(1, &m_vao);
    }
    if (m_vbo != 0)
    {
        glDeleteBuffers(1, &m_vbo);
    }
}

void FullscreenQuad::Draw() const
{
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}