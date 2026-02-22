#pragma once

#include <string>
#include <map>

// On inclut les classes de notre module Math pour les utiliser dans les fonctions "set"
#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"
#include "Math/Matrix3x3.h"
#include "Math/Vector2.h"

/**
 * @brief Gère la compilation et la liaison d'un programme shader OpenGL à partir de fichiers
 * @details Cette classe encapsule un programme shader OpenGL (Vertex + Fragment)
 * Elle lit le code GLSL depuis les fichiers, le compile, le lie et fournit des fonctions pour définir des variables "uniform"
 *
 */
class Shader
{
public:
    // interdiction de copie / déplacer 
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;
    Shader(Shader &&) = delete;
    Shader &operator=(Shader &&) = delete;

    /**
     * @brief l'ID du programme OpenGL
     *
     */
    unsigned int ID;

    /**
     * @brief Constructeur qui lit et construit le shader
     * @details accepte un troisième fichier optionnel geometry shader
     *
     * @param vertexPath le chemin vers le fichier du vertex shader (.vert)
     * @param fragmentPath le chemin vers le fichier du fragment shader (.frag)
     * @param geometryPath le chemin vers le fichier du fragment shader (.geom)
     */
    Shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath = nullptr);

    // le destructeur sera nécessaire pour libérer le programm de la VRAM
    ~Shader();

    /**
     * @brief Active le programme shader pour le rendu
     * @details Tout ce qui sera dessiné après cet appel utilisera ce shader
     */
    void Use();

    // --- fonctions d'aide pour les Uniforms ---
    // Les uniforms sont des variables globales dans les shaders qu'on peut définir depuis notre code C++

    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec3(const std::string &name, const Vector3 &vec) const;
    void setVec2(const std::string &name, const Vector2 &vec) const;
    void setMat4(const std::string &name, const Matrix4x4 &mat) const;
    void setMat3(const std::string &name, const Matrix3x3 &mat) const;
    void setUInt(const std::string &name, const unsigned int value) const;

private:
    /**
     * @brief Cache pour les locations d'uniforms.
     * @details le mot-clé "mutable" nous permet de modifier ce cache depuis une méthode const (comme nos fonctions set...)
     * car la modification du cache est une optimisation et non un changement de l'état logique de l'objet Shader.
     *
     */
    mutable std::map<std::string, int> m_uniformLocationCache;

    // Chemins des fichiers shader pour msg erreur
    std::string m_vertexPath;
    std::string m_fragmentPath;
    std::string m_geometryPath;

    /**
     * @brief Fonction utilitaire privée pour vérifier les erreurs de compilation/liaison
     *
     * @param shader id du shader/programme à vérifier
     * @param type type à vérifier (vertex, fragment ou program)
     * @param filePath chemin de fichier source
     */
    void checkCompileErrors(unsigned int shader, std::string type, const char *filePath);

    /**
     * @brief Récupère la location d'un uniform, en utilisant le cache
     *
     * @param name le nom de l'uniform dans le code GLSL
     * @return int id la location de l'uniform
     */
    int getUniformLocation(const std::string &name) const;
};