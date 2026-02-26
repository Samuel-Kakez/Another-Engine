#include "Renderer/Geometry/Mesh.h"
#include "Debug/StatsManager.h"

Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices)
{
    this->indexCount = static_cast<unsigned int>(indices.size());
    setupMesh(vertices, indices);
}

void Mesh::setupMesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices)
{
    // --- 0. Calcul de l'AABB locale --
    // On parcourt tous les sommets du maillage pour trouver les étentudes min et max
    for (const auto &vertex : vertices)
    {
        aabb.Extend(vertex.Position);
    }

    // --- 1. creation des identifiants pour nos buffers ---
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // --- 2. configuration et envoi des données ---

    // On active notre VAO pour que toutes les configurations suivantes y soient enregistrées

    glBindVertexArray(VAO);

    // configuration du VBO (données de sommets)

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(Vertex),
                 vertices.empty() ? nullptr : vertices.data(),
                 GL_STATIC_DRAW);

    // configuration de l'EBO (données des indices)

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.empty() ? nullptr : indices.data(),
                 GL_STATIC_DRAW);

    // --- 3. Définition des attributs de sommets ---

    // attribut de position (location = 0)
    // active l'attribut à la loc 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Position));

    // attribut de la normale (location = 1)
    // active l'attribut à la loc 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));

    // attribut de coords texture (location = 2)
    // active l'attribut à la loc 2
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));

    // attribut pour la tangente (loc 3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));

    // attribut pour la bitangente (loc 4)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));

    // une fois la config terminée, on délie le VAO pour éviter toute modif accidentelle
    glBindVertexArray(0);
}

void Mesh::Draw()
{
    // Enregistre cet appel de dessinn (DEBUG)
    // indexCount contient le nombre de sommets. On /3 pour avoir le nombre de triangles.
    StatsManager::LogDrawCall(indexCount / 3);
    // active le VAO
    glBindVertexArray(VAO);
    // appel de dessin
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

// destructeur : supprime VAO, VBO et EBO
Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}