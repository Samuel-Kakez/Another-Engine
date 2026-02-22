#pragma once

#include "Core/Component.h"
#include "Math/Vector3.h"

/// @class DirectionalLight
/// @brief Composant représentant une lumière directionnelle

/// Une Lumière Directionnelle éclaire toute la scène depuis une direction donnée sans atténuation par la distance (comme le Soleil)
/// La direction de la lumière est déterminée par le vecteur Forward du Transform de la lumière
class DirectionalLight : public Component
{

public:
	/// @brief Indique si cette lumière projette des ombres
	bool castsShadows;
	/// @brief Résolution de la shadowmap en pixels
	unsigned int shadowResolution;
	/// @brief Identifiant OpenGL de la shadow map (0 si non initialisé)
	unsigned int shadowMap = 0;
	/// @brief Constructeur par défaut
	DirectionalLight();
	/// @param intensity 
	/// @param castsShadows 
	/// @param shadowResolution 
	DirectionalLight(bool castsShadows = true, unsigned int shadowResolution = 2048);
	/// @brief Destructeur par défaut
	~DirectionalLight() = default;
};