#pragma once

#include <numbers>
namespace Math
{
    /**
     * @brief Constante PI de haute précision pour les calculs mathématiques
     * @details Utilise la constante standard C++20 pour une précision maximale
     * inline constexpr permet de définir la constante directement dans le header de manière sûre et efficace, en garantissant une seule définition à travers tout le projet
     *
     */
    inline constexpr float PI = static_cast<float>(std::numbers::pi);
}