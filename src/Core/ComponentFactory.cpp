#include "Core/ComponentFactory.h"
#include "Debug/Logger.h"

ComponentFactory &ComponentFactory::Instance()
{
    // L'instance unique est créée en tant que variable statique locale.
    // Elle est initialisée au premier appel de Instance() et détruite à la fin du programme.
    static ComponentFactory instance;
    return instance;
}

void ComponentFactory::RegisterComponent(const std::string &typeName, ComponentCreator creator)
{
    LOG_INFO("type de composant '%s' enregistré dans la factory.", typeName.c_str());
    // On associe le nom du type à sa fonction de création dans la map
    m_creators[typeName] = creator;
}

Component *ComponentFactory::CreateComponent(const std::string &typeName, GameObject *owner, const nlohmann::json &data)
{
    // On cherche si un créateur a été enregistré pour ce nom de type
    auto it = m_creators.find(typeName);
    if (it != m_creators.end())
    {
        // si oui, on appelle la fonction de création stockée
        return it->second(owner, data);
    }
    // si non, on affiche une erreur et on retourne un pointeur nul
    LOG_ERROR("type inconnu '%s'.", typeName.c_str());
    return nullptr;
}