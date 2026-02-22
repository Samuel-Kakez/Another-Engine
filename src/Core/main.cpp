#include "Core/Engine.h"

// Indication pour forcer l'utilisation du GPU dédié
// Sur les systèmes multi-GPU (portables avec NVIDIA Optimus),
// Ce code "exporte" des symboles que les pilotes NVIDIA et AMD recherchent au lancement
// de l'exécutable pour sélectionner le GPU haute performance.

#ifdef _WIN32
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
int main()
{
    // Créé une instance du moteur sur la pile
    Engine engine;

    // Lance la boucle de jeu
    engine.Run();

    return 0;
} // L'objet "engine" est détruit ici, son destructeur est appelé, et tout est nettoyé