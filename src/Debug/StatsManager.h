#pragma once

/**
 * @brief Classe statique pour collecter les statistiques de rendu par frame.
 * @details Permet un accès global simple pour incrémenter les compteurs
 * depuis n'importe où dans le code de rendu.
 *
 */
class StatsManager
{
public:
    static unsigned int drawCalls;
    static unsigned int triangles;
    static unsigned int materialBinds;

    /**
     * @brief Remet à zéro les compteurs. On l'appelle à chaque frame.
     *
     */
    static void Reset()
    {
        drawCalls = 0;
        triangles = 0;
        materialBinds = 0;
    }

    /**
     * @brief Incrémente les compteurs pour un appel de dessin
     *
     * @param triangleCount le nombre de triangles dessinés cet appel
     */
    static void LogDrawCall(unsigned int triangleCount)
    {
        drawCalls++;
        triangles += triangleCount;
    }

    /**
     * @brief Incrémente le compteur de matériaux batchés
     * 
     */
    static void LogMaterialBind()
    {
        materialBinds++;
    }
};