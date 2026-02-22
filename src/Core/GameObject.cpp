#include "Core/GameObject.h"
#include "Core/Component.h"
#include <typeindex>

// le constructeur est vide, les membres s'initialisent automatiquement
GameObject::GameObject(const std::string &name) : name(name)
{
}

// destructeur inutile : les unique_ptr s'en occupent
GameObject::~GameObject()
{
}

// la fonction update propage l'appel à tous les composants
void GameObject::Update(float deltaTime)
{
    for (const auto &component : m_components)
    {
        component->Update(deltaTime);
    }
}

void GameObject::FixedUpdate(float fixedDeltaTime)
{
    for (const auto &component : m_components)
    {
        component->FixedUpdate(fixedDeltaTime);
    }
}