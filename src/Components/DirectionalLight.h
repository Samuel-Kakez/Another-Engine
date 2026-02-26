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

	/// @brief Nombre de cascades pour le CSM
	static constexpr int NUM_CASCADES = 4;

	/// @brief Indique si cette lumière projette des ombres
	bool castsShadows;
	/// @brief Résolution de la shadowmap en pixels
	unsigned int shadowResolution;
	
	/// @brief Identifiant OpenGL de la shadow map array (GL_TEXTURE_2D_ARRAY, NUM_CASCADES layers)
	/// Initialisé à 0, créé par LightManager::CreateShadowResourcesFor()
	unsigned int shadowMapArray = 0;

	/// @param intensity 
	/// @param castsShadows 
	/// @param shadowResolution 
	DirectionalLight(bool castsShadows = true, unsigned int shadowResolution = 2048);
	/// @brief Destructeur par défaut
	~DirectionalLight() = default;
};