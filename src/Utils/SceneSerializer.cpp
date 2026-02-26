#include "Utils/SceneSerializer.h"
#include "Core/Scene.h"
#include "Core/GameObject.h"
#include "Managers/ResourceManager.h"
#include "Components/Camera.h"
#include "Core/ComponentFactory.h"
#include "Core/Transform.h"
#include "Renderer/Material.h"
#include "Debug/Logger.h"

#include <fstream>
#include <nlohmann/json.hpp>

// pour un accès plus facile à l'espace de nom de la bibliothèque JSON
using json = nlohmann::json;

static Vector3 ParseVector3FromJSON(const json &jsonData, const Vector3 &defaultValue = {0.0f, 0.0f, 0.0f})
{
    if (jsonData.is_array() && jsonData.size() == 3)
    {
        return Vector3(jsonData[0].get<float>(), jsonData[1].get<float>(), jsonData[2].get<float>());
    }
    return defaultValue;
}

static Vector2 ParseVector2FromJSON(const json &jsonData, const Vector2 &defaultValue = {0.0f, 0.0f})
{
    if (jsonData.is_array() && jsonData.size() == 2)
    {
        return Vector2(jsonData[0].get<float>(), jsonData[1].get<float>());
    }
    return defaultValue;
}

SceneSerializer::SceneSerializer(Scene &scene) : m_scene(scene)
{
}

bool SceneSerializer::Deserialize(const std::string &filepath)
{
    // 1. lire le fichier
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        LOG_ERROR("impossible d'ouvrir le fichier de scène '%s'.", filepath.c_str());
        return false;
    }

    // 2. parser le json
    json data;
    try
    {
        data = json::parse(file);
    }
    catch (json::parse_error &e)
    {
        LOG_ERROR("JSON invalide pendant la désérialisation - %s", e.what());
        return false;
    }
    catch (const json::exception &e)
    {
        LOG_ERROR("Erreur JSON pendant la désérialisation - %s", e.what());
        return false;
    }

    LOG_INFO("fichier '%s' parsé avec succès.", filepath.c_str());

    // 3. Préparer la scène et les managers
    m_scene.Clear();
    ResourceManager &resourceManager = m_scene.GetResourceManager();

    // 4. Parser la section "materials".

    if (data.contains("materials") && data["materials"].is_array())
    {
        for (const auto &materialData : data["materials"])
        {
            std::string name = materialData.value("name", "");
            std::string shaderName = materialData.value("shader", "");
            if (name.empty() || shaderName.empty())
            {
                LOG_WARN("matériau ignoré (nom ou shader manquant).");
                continue;
            }

            Shader *shader = resourceManager.GetShader(shaderName);
            if (!shader)
            {
                LOG_ERROR("shader '%s' introuvable pour le matériau '%s'.", shaderName.c_str(), name.c_str());
                continue;
            }

            // Crée ou Récupère le matériau s'il existe
            std::shared_ptr<Material> material = resourceManager.GetOrCreateMaterial(name, shader);

            // Parse les propriétés PBR
            if (materialData.contains("albedoColor"))
            {
                material->albedoColor = ParseVector3FromJSON(materialData["albedoColor"], {1.0f, 1.0f, 1.0f});
            }

            material->metallic = materialData.value("metallic", 0.0f);
            material->roughness = materialData.value("roughness", 0.5f);

            if (materialData.contains("tiling"))
            {
                material->tiling = ParseVector2FromJSON(materialData["tiling"], {1.0f, 1.0f});
            }

            if (materialData.contains("albedoTexture"))
            {
                std::string texturePath = materialData["albedoTexture"];
                material->albedoTexture = resourceManager.GetTexture(texturePath, texturePath, true);
            }

            if (materialData.contains("normalTexture"))
            {
                std::string texturePath = materialData["normalTexture"];
                material->normalTexture = resourceManager.GetTexture(texturePath, texturePath, false);
            }
            material->normalMapIntensity = materialData.value("normalMapIntensity", 1.0f);
        }
    }

    // 5. Parser la section gameObjects
    std::vector<std::pair<std::string, std::string>> pendingParentLinks;
    if (data.contains("gameObjects") && data["gameObjects"].is_array())
    {
        for (const auto &goData : data["gameObjects"])
        {
            // On lit le nom depuis le JSON, ou on utilise un nom par défaut.
            std::string goName = goData.value("name", "Unnamed GameObject");
            GameObject *newGameObject = m_scene.AddGameObject(goName);

            if (goData.contains("components") && goData["components"].is_array())
            {
                for (const auto &compData : goData["components"])
                {
                    std::string type = compData.value("type", "");
                    if (!type.empty())
                    {
                        // Un seul appel
                        ComponentFactory::Instance().CreateComponent(type, newGameObject, compData);
                    }
                }
            }

            if (goData.contains("parent"))
            {
                std::string parentName = goData.value("parent", "");
                if (!parentName.empty())
                {
                    pendingParentLinks.push_back({goName, parentName});
                    LOG_TRACE("lien parent en attente ajouté : enfant='%s', parent='%s'.", goName.c_str(), parentName.c_str());
                }
            }
        }
    }

    LOG_TRACE("résolution des parentés en attente (%zu liens).", pendingParentLinks.size());

    for (const auto &[childName, parentName] : pendingParentLinks)
    {
        GameObject *childGO = m_scene.FindGameObjectByName(childName);
        GameObject *parentGO = m_scene.FindGameObjectByName(parentName);

        if (!childGO || !parentGO)
        {
            LOG_WARN("parenté impossible - parent '%s' ou enfant '%s' introuvable", parentName.c_str(), childName.c_str());
            continue;
        }

        Transform *childTransform = childGO->GetComponent<Transform>();
        Transform *parentTransform = parentGO->GetComponent<Transform>();

        if (childTransform && parentTransform)
        {
            childTransform->SetParent(parentTransform);
        }
        else
        {
            LOG_WARN("parenté impossible - Transform manquant sur '%s' ou '%s'", childName.c_str(), parentName.c_str());
        }
    }
    size_t goCount = data.contains("gameObjects") ? data["gameObjects"].size() : 0;
    LOG_INFO("scène chargée - %zu GameObjects créés.", goCount);

    LOG_TRACE("flush d'initialisation des objets après chargement de la scène.");
    m_scene.FlushPendingInitialization();
    return true;
}