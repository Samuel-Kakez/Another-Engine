#include "Renderer/Shader.h"
#include "Debug/Logger.h"

#include <fstream>
#include <sstream>
#include <glad/glad.h>

Shader::Shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath)
    : m_vertexPath(vertexPath ? vertexPath : "N/A"),
      m_fragmentPath(fragmentPath ? fragmentPath : "N/A"),
      m_geometryPath(geometryPath ? geometryPath : "N/A")
{

    const bool hasGeometryShader = (geometryPath != nullptr);

    // 1. Récupérer le code source des shaders depuis les fichiers

    // d'abord, on déclare les variables qui contiendront le code final

    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;

    // on déclare nos "mains" pour ouvrir les fichiers

    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;

    // on active une sécurité sur nos "mains" : si elles n'arrivent pas à
    // ouvrir le fichier ou à le lire, elles lèvent une exception que le bloc catch attrapera
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    if (hasGeometryShader)
    {
        gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    }

    // démarrage du bloc try catch
    try
    {
        // on demande aux nos mains d'ouvrir les fichiers
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);

        // on prépare nos bloc-notes temporaires
        std::stringstream vShaderStream, fShaderStream;

        // ligne la plus importante :
        // on verse tout le contenu du fichier (vShaderFile.rdbuf())
        // d'un seul coup dans notre bloc-notes (vShaderStream)
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        // maintenant que c'est dans le bloc-notes, on ferme les livres
        vShaderFile.close();
        fShaderFile.close();

        // on demande à nos bloc-notes de donner tout leur contenu sous forme de string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        // si on a un geometry shader fourni
        if (hasGeometryShader)
        {
            gShaderFile.open(geometryPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    }
    catch (std::ifstream::failure &e)
    {
        LOG_ERROR("impossible de lire les fichiers source (vertex: %s, fragment: %s%s) - %s", vertexPath, fragmentPath, hasGeometryShader ? (std::string(", geometry: ") + geometryPath).c_str() : "", e.what());
    }

    // on récupère le code source sous forme de chaîne de caractère C
    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();

    // 2. Compilation du Vertex Shader
    unsigned int vertex;
    // on demande à openGL un ID pour un nouvel objet vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    // on attache notre code source à cet ID
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    // on demande à openGL de compiler le code
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX", vertexPath);

    // 3. Compilation du fragment Shader
    unsigned int fragment;
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT", fragmentPath);

    // 4. Tentative de compilation du geometry shader si fourni
    unsigned int geometry;
    if (hasGeometryShader)
    {
        const char *gShaderCode = geometryCode.c_str();
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        checkCompileErrors(geometry, "GEOMETRY", geometryPath);
    }

    // Lier les shaders dans un programme

    ID = glCreateProgram();

    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    if (hasGeometryShader)
    {
        glAttachShader(ID, geometry);
    }
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM", "N/A");

    // Supprimer les shaders car ils sont liés désormais
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (hasGeometryShader)
    {
        glDeleteShader(geometry);
    }
}

void Shader::checkCompileErrors(unsigned int shader, std::string type, const char *filePath)
{
    int success;
    char infoLog[1024]; // un buffer pour stocker le message d'erreur

    // si c'est une erreur de shader
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            LOG_ERROR("échec de compilation (%s, fichier : %s)\n%s", type.c_str(), (filePath ? filePath : "inconnu"), infoLog);
        }
    }
    else // si c'est une erreur de programme
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            LOG_ERROR("échec de l'édition de liens (vertex: %s, fragment: %s%s)\n%s", m_vertexPath.c_str(), m_fragmentPath.c_str(), m_geometryPath.empty() ? "" : (", geometry: " + m_geometryPath).c_str(), infoLog);
        }
    }
}

int Shader::getUniformLocation(const std::string &name) const
{
    // On cherche d'abord dans le cache
    if (m_uniformLocationCache.count(name))
    {
        return m_uniformLocationCache.at(name);
    }
    // Si non trouvé, on appelle OpenGL et on stocke le résultat dans le cache
    int location = glGetUniformLocation(ID, name.c_str());
    if (location == -1)
    {
        LOG_WARN("variable uniform '%s' introuvable.", name.c_str());
    }
    m_uniformLocationCache[name] = location;
    return location;
}

void Shader::Use()
{
    glUseProgram(ID);
}

void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(getUniformLocation(name), (int)value);
}

void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setVec3(const std::string &name, const Vector3 &vec) const
{
    glUniform3f(getUniformLocation(name), vec.x, vec.y, vec.z);
}
void Shader::setVec2(const std::string &name, const Vector2 &vec) const
{
    glUniform2f(getUniformLocation(name), vec.x, vec.y);
}

void Shader::setMat4(const std::string &name, const Matrix4x4 &mat) const
{
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, mat.m);
}

void Shader::setMat3(const std::string &name, const Matrix3x3 &mat) const
{
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, mat.m);
}

void Shader::setUInt(const std::string &name, const unsigned int value) const
{
    glUniform1ui(getUniformLocation(name), value);
}

Shader::~Shader()
{
    glDeleteProgram(ID);
}