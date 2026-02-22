#include "Components/MeshRenderer.h"
#include "Core/ComponentFactory.h"
#include "Core/GameObject.h"
#include "Core/Scene.h"
#include "Managers/ResourceManager.h"

#include <nlohmann/json.hpp>
#include "Debug/Logger.h"

// constructeur
MeshRenderer::MeshRenderer(Mesh *mesh, std::shared_ptr<Material> material) : mesh(mesh), material(std::move(material)) {}

// on utilise un namespace anonyme pour que ces fonctions ne soit visibles que depuis ce fichier
namespace
{
    Component *CreateMeshRenderer(GameObject *owner, const nlohmann::json &data)
    {
        std::string modelPath = data.value("model", "");
        if (modelPath.empty())
        {
            return nullptr;
        }

        ResourceManager &resourceManager = owner->m_ownerScene->GetResourceManager();
        Model *model = resourceManager.GetModel(modelPath);
        if (!model || model->meshes.empty())
        {
            return nullptr;
        }

        std::string materialName = data.value("material", "");
        if (materialName.empty())
        {
            LOG_ERROR("aucun matériau spécifié pour l'objet '%s'.", owner->name.c_str());
            return nullptr;
        }

        std::shared_ptr<Material> material = resourceManager.GetMaterial(materialName);
        if (!material)
        {
            LOG_ERROR("matériau '%s' introuvable.", materialName.c_str());
            return nullptr;
        }

        // ajoute un MeshRenderer par mesh dans le model
        MeshRenderer *firstMR = nullptr;
        for (const auto &meshPtr : model->meshes)
        {
            MeshRenderer *mr = owner->AddComponent<MeshRenderer>(meshPtr.get(), material);
            if (!firstMR)
            {
                firstMR = mr;
            }
        }
        return firstMR; // retourne le premier MR pour éviter un fail
    }

    // Cette variable statique est initialisée une seule fois au lancement du programme
    // La lambda est immédiatement exécutée, ce qui appelle RegisterComponent.
    const bool isMeshRendererRegistererd = []
    {
        ComponentFactory::Instance().RegisterComponent("MeshRenderer", CreateMeshRenderer);
        return true;
    }();
}