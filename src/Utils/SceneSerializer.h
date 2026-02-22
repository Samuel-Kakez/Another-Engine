#pragma once

#include <string>

// déclaration anticipée de la classe Scene pour éviter une dépendance circulaire
class Scene;

/**
 * @brief Gère la sérialisation et la désérialisation des scènes.
 * @details Pour l'instant, elle se concentre sur le chargement (désérialisation) d'une scène à partir d'un fichier au format JSON.
 *
 */
class SceneSerializer
{
public:
    /**
     * @brief Constructeur
     *
     * @param scene la référence vers la scène à peupler
     */
    SceneSerializer(Scene &scene);

    /**
     * @brief Charge et parse un fichier de scène JSON pour remplir l'objet Scene.
     *
     * @param filepath le chemin d'accès vers le fichier de scène (.json)
     * @return true si le chargement et le parsing ont réussi
     * @return false sinon
     */
    bool Deserialize(const std::string &filepath);

private:
    // référence vers la scène qu'on doit modifier
    Scene &m_scene;
};