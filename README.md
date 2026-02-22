# Another Engine
Petit moteur C++ / OpenGL 3.3 open-source, encore très tôt dans son développement (early stage). L’objectif est d’explorer un pipeline de rendu moderne et une architecture scène / composants.

## Points clés
- Boucle de jeu à pas de temps fixe, rendu découplé du taux d’update.
- Chargement de scènes JSON via `SceneSerializer` (`main.scene.json`).
- Enregistrement automatique des objets rendables (`MeshRenderer`) via événements.
- Cache centralisé pour shaders, textures, modèles et matériaux.
- Pipeline multipasse (ombres + lighting) avec UBO.
- UI debug intégrée (ImGui) avec stats et logs.

## Stack technique
- OpenGL 3.3 Core, GLFW, GLAD
- ImGui, Assimp, stb, nlohmann_json

## Statut
- Projet open-source
- API et comportements encore instables, changements fréquents.

## Licence
MIT
